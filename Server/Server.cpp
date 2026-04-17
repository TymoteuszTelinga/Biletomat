
#include <iostream>
#include <chrono>
#include <format>

#include "Server.h"

#include <fstream>

//conan install . --output-folder=out/build/conan --build=missing -s build_type=Debug

void DisplayServerState(const std::unordered_map<uint32_t, TicketData>& Tickets, const std::unordered_map<std::string, BookingEntry>& bookings)
{
    std::cout << "Tickets\n";
    for (const auto& [id, ticket] : Tickets)
    {
        std::string s = std::format("ID:{} | Name:{} | Count:{}", id, ticket.Name, ticket.Count);
        std::cout << s << std::endl;
    }

    std::cout << "Bookings\n";
    for (const auto& [id, booking] : bookings)
    {
        std::string s = std::format("ID:{} | TicketID:{}", id, booking.TicketID);
        std::cout << s << std::endl;
    }
}

TicketServer::TicketServer()
    :m_TimeoutManager(m_BookedTickets, m_BookedMutex)
{
    m_Svr.Get("/events", [this](const httplib::Request& req, httplib::Response& res)
        {
            OnEvents(req, res);
        });

    m_Svr.Get("/tickets", [this](const httplib::Request& req, httplib::Response& res) 
        {
            OnGetTickets(req, res);
        });

    m_Svr.Post("/book", [&](const httplib::Request& req, httplib::Response& res)
        {
            OnBook(req, res);
        });

    m_Svr.Post("/cancel", [&](const httplib::Request& req, httplib::Response& res)
        {
            OnCancel(req, res);
        });

    m_Svr.Post("/confirm", [&](const httplib::Request& req, httplib::Response& res)
        {
            OnConfirm(req, res);
        });

    m_TimeoutManager.SetTimeout(60.f);
    m_TimeoutManager.SetCallback([this](const std::vector<std::string>& expiredBookings)
        {
            OnBookingsExpired(expiredBookings);
        });
}

TicketServer::~TicketServer()
{
    m_TimeoutManager.Stop();
}

void TicketServer::LoadData(const std::filesystem::path& dataPath)
{
    std::ifstream file("tickets.txt");
    if (file.is_open())
    {
        nlohmann::json data;
        file >> data;
        m_AvailableTickets = data;
    }
    else
    {
        std::cout << "Error opening tickets.txt file\n";
    }
}

void TicketServer::Run()
{
    m_TimeoutManager.Start(5);
    std::cout << "Server running on http://localhost:8080\n";
    m_Svr.listen("localhost", 8080);
}

void TicketServer::OnEvents(const httplib::Request& req, httplib::Response& res)
{
    std::string clientID = req.get_param_value("client_id");
    std::cout << "ClientID " << clientID << std::endl;

    res.set_header("Content-Type", "text/event-stream");
    res.set_header("Cache-Control", "no-cache");
    res.set_header("Connection", "keep-alive");
    //res.set_header("Access-Control-Allow-Origin", "*");
    res.set_chunked_content_provider("text/event-stream",
        [&, clientID](size_t offset, httplib::DataSink& sink) -> bool
        {
            std::cout << "ImOpen\n";
            m_Broadcaster.AddClient(clientID, sink);

            std::string ping = "event: connected\ndata: {}\n\n";
            sink.write(ping.c_str(), ping.size());

            while (sink.is_writable())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            m_Broadcaster.RemoveClient(clientID);
            return true;
        }
    );
}

void TicketServer::OnGetTickets(const httplib::Request& req, httplib::Response& res)
{
    nlohmann::json ticketsData = m_AvailableTickets;
    res.set_content(ticketsData.dump(), "application/json");
}

void TicketServer::OnBook(const httplib::Request& req, httplib::Response& res)
{
    ClientRequest request = nlohmann::json::parse(req.body);
    std::cout << request.ClientID << "Want to book" << request.TicketID << std::endl;

    uint32_t responseID = -1;
    auto it = m_AvailableTickets.find(request.TicketID);
    if (it != m_AvailableTickets.end())
    {
        if (it->second.Count > 0)
        {
            std::lock_guard<std::mutex> lock(m_BookedMutex);
            BookingEntry entry;
            entry.ClientID = request.ClientID;
            responseID = entry.TicketID = request.TicketID;
            entry.BookedTime = std::chrono::steady_clock::now();
            m_BookedTickets[entry.ClientID] = entry;
            it->second.Count--;

            m_Broadcaster.Broadcast("TickedBooked", { { "tickedID", request.TicketID } });
        }
    }
    //DisplayServerState(AvailableTickets, BookedTickets);
    nlohmann::json response = { {"id", responseID} };
    res.set_content(response.dump(), "application/json");
}

void TicketServer::OnCancel(const httplib::Request& req, httplib::Response& res)
{
    ClientRequest request = nlohmann::json::parse(req.body);

    //cancel Booking
    RemoveBooking(request.ClientID);

    //return BookedTicket
    {
        auto it = m_AvailableTickets.find(request.TicketID);
        if (it != m_AvailableTickets.end())
        {
            m_AvailableTickets[request.TicketID].Count++;
            m_Broadcaster.Broadcast("TickedAvailable", { { "tickedID", request.TicketID } });
        }
    }

    //DisplayServerState(AvailableTickets, BookedTickets);
    res.set_content(req.body, "application/json");
}

void TicketServer::OnConfirm(const httplib::Request& req, httplib::Response& res)
{
    BookingData data = nlohmann::json::parse(req.body);
    //update purchase history
    {
        History soldTicket;
        soldTicket.Name = data.Name;
        soldTicket.Surname = data.Surname;
        soldTicket.TicketID = data.TicketID;
        soldTicket.ClientID = data.ClientID;
        m_SoldTickets.push_back(soldTicket);
    }
    //if ticket was booked remove booking
    RemoveBooking(data.ClientID);

    //DisplayServerState(AvailableTickets, BookedTickets);
    res.set_content(req.body, "application/json");//dummy/potencly to remove
}

void TicketServer::RemoveBooking(const std::string& clientID)
{
    std::lock_guard<std::mutex> lock(m_BookedMutex);
    auto it = m_BookedTickets.find(clientID);
    if (it != m_BookedTickets.end())
    {
        m_BookedTickets.erase(clientID);
    }
}

void TicketServer::OnBookingsExpired(const std::vector<std::string>& expiredBookings)
{
    std::lock_guard<std::mutex> lock(m_BookedMutex);
    for (size_t i = 0; i < expiredBookings.size(); i++)
    {
        //cancel Booking
        uint32_t ticketID = -1;
        {
            auto it = m_BookedTickets.find(expiredBookings[i]);
            if (it != m_BookedTickets.end())
            {
                ticketID = it->second.TicketID;
                m_BookedTickets.erase(expiredBookings[i]);
                m_Broadcaster.SentToClient(expiredBookings[i], "TicketTimeout", { {} });
            }
        }

        //return BookedTicket
        if (ticketID != -1)
        {
            auto it = m_AvailableTickets.find(ticketID);
            if (it != m_AvailableTickets.end())
            {
                m_AvailableTickets[ticketID].Count++;
                m_Broadcaster.Broadcast("TickedAvailable", { { "tickedID", ticketID } });
            }
        }
    }
}
