
#include <iostream>
#include <chrono>
#include <format>

#include <httplib/httplib.h>

#include "../Common/StructDefiicions.h"
#include "BroadcastHub.h"
#include "TimeoutManager.h"
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

void LoadTickets(std::unordered_map<uint32_t, TicketData>& tickets)
{
    std::ifstream file("tickets.txt");
    if (file.is_open())
    {
        nlohmann::json data;
        file >> data;
        tickets = data;
    }
    else
    {
        std::cout << "Error opening tickets.txt file\n";
    }
}

int main()
{  
    httplib::Server svr;

    std::unordered_map<uint32_t, TicketData> AvailableTickets;
    //AvailableTickets[0] = { 8, 170, "Ulgowy"};
    //AvailableTickets[1] = { 3, 270, "Stundencki"};
    //AvailableTickets[2] = { 2, 370, "Senior"};
    //AvailableTickets[3] = { 5, 470, "Normalny"};

    LoadTickets(AvailableTickets);

    std::vector<History> SoldTickets;

    std::unordered_map<std::string, BookingEntry> BookedTickets;
    std::mutex BookedMutex;

    Broadcaster broadcaster;
    TimeoutManager timeoutManager(BookedTickets, BookedMutex);
    timeoutManager.SetTimeout(60.f);
    timeoutManager.SetCallback([&](const std::vector<std::string>& expiredBookings) 
        {
            std::lock_guard<std::mutex> lock(BookedMutex);
            for (size_t i = 0; i < expiredBookings.size(); i++)
            {
                //cancel Booking
                uint32_t ticketID = -1;
                {
                    auto it = BookedTickets.find(expiredBookings[i]);
                    if (it != BookedTickets.end())
                    {
                        ticketID = it->second.TicketID;
                        BookedTickets.erase(expiredBookings[i]);
                        broadcaster.SentToClient(expiredBookings[i], "TicketTimeout", { {} });
                    }
                }

                //return BookedTicket
                if (ticketID != -1)
                {
                    auto it = AvailableTickets.find(ticketID);
                    if (it != AvailableTickets.end())
                    {
                        AvailableTickets[ticketID].Count++;
                        broadcaster.Broadcast("TickedAvailable", { { "tickedID", ticketID } });
                    }
                }
            }
        });
    
    svr.Get("/events", [&](const httplib::Request& req, httplib::Response& res) 
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
                    broadcaster.AddClient(clientID, sink);

                    std::string ping = "event: connected\ndata: {}\n\n";
                    sink.write(ping.c_str(), ping.size());

                    while (sink.is_writable())
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    }
                    broadcaster.RemoveClient(clientID);
                    return true;
                }
            );
        });

    svr.Get("/tickets", [&](const httplib::Request& req, httplib::Response& res)
        {
            nlohmann::json ticketsData = AvailableTickets;
            res.set_content(ticketsData.dump(), "application/json");
        });

    // POST /book{idRequest} -> { idRequest : -1 if failed}
    svr.Post("/book", [&](const httplib::Request& req, httplib::Response& res) 
        {
        //std::lock_guard<std::mutex> lock(TicketsMutex);
        ClientRequest request = nlohmann::json::parse(req.body);
        std::cout << request.ClientID << "Want to book" << request.TicketID << std::endl;

        uint32_t responseID = -1;
        auto it = AvailableTickets.find(request.TicketID);
        if (it != AvailableTickets.end())
        {
            if (it->second.Count > 0)
            {
                std::lock_guard<std::mutex> lock(BookedMutex);
                BookingEntry entry;
                entry.ClientID = request.ClientID;
                responseID = entry.TicketID = request.TicketID;
                entry.BookedTime = std::chrono::steady_clock::now();
                BookedTickets[entry.ClientID] = entry;
                it->second.Count--;

                broadcaster.Broadcast("TickedBooked", { { "tickedID", request.TicketID } });
            }
        }

        DisplayServerState(AvailableTickets, BookedTickets);

        nlohmann::json response = { {"id", responseID} };
        res.set_content(response.dump(), "application/json");
        //Ticket& ticket = Tickets[request.TicketID];
        //uint32_t responseID = -1;
        //if (ticket.State == TicketState::AVAILABLE)
        //{
        //    ticket.State = TicketState::BOOKED;
        //    ticket.Booking.ClientID = request.ClientID;
        //    ticket.Booking.TicketID = request.TicketID;
        //    ticket.Booking.BookedTime = std::chrono::steady_clock::now();
        //    responseID = request.TicketID;

        //    broadcaster.Broadcast("TickedBooked", { { "tickedID", request.TicketID } });//BroadCast data
        //}
        //DisplayTicketsStates(Tickets);

        /*nlohmann::json response = {{"id", responseID}};
        res.set_content(response.dump(), "application/json");*/
        });

    // POST /cancel{orderId} 
    svr.Post("/cancel", [&](const httplib::Request& req, httplib::Response& res)
        {
        ClientRequest request = nlohmann::json::parse(req.body);

        //cancel Booking
        {
            std::lock_guard<std::mutex> lock(BookedMutex);
            auto it = BookedTickets.find(request.ClientID);
            if (it != BookedTickets.end())
            {
                BookedTickets.erase(request.ClientID);
            }
        }

        //return BookedTicket
        {
            auto it = AvailableTickets.find(request.TicketID);
            if (it != AvailableTickets.end())
            {
                AvailableTickets[request.TicketID].Count++;
                broadcaster.Broadcast("TickedAvailable", { { "tickedID", request.TicketID } });
            }
        }

        DisplayServerState(AvailableTickets, BookedTickets);
        res.set_content(req.body, "application/json");//dummy/potencly to remove
        });

    // POST /confirm{BookingData}
    svr.Post("/confirm", [&](const httplib::Request& req, httplib::Response& res)
        {
            BookingData data = nlohmann::json::parse(req.body);
            //update purchase history
            {
                History soldTicket;
                soldTicket.Name = data.Name;
                soldTicket.Surname = data.Surname;
                soldTicket.TicketID = data.TicketID;
                soldTicket.ClientID = data.ClientID;
                SoldTickets.push_back(soldTicket);
            }
            //if ticket was booked remove booking
            {
                std::lock_guard<std::mutex> lock(BookedMutex);
                auto it = BookedTickets.find(data.ClientID);
                if (it != BookedTickets.end())
                {
                    BookedTickets.erase(data.ClientID);
                }
            }

            DisplayServerState(AvailableTickets, BookedTickets);
            res.set_content(req.body, "application/json");//dummy/potencly to remove
        });

    std::cout << "Server running on http://localhost:8080\n";

    timeoutManager.Start(5);
    svr.listen("localhost", 8080);
    timeoutManager.Stop();

	return 0;
}