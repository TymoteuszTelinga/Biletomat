
#include "Biletomat.h"
#include "../Common/StructDefiicions.h"

BiletomatCli::BiletomatCli()
{
    srand(time(nullptr));
    uint32_t id = rand();
    m_MachineID = std::format("machine{}", id);
    m_Cli = std::make_unique<httplib::Client>("localhost", 8080);
    m_SSEClient = std::make_unique< httplib::sse::SSEClient>(*m_Cli.get(), "/events?client_id=" + m_MachineID);

    m_Tickets.clear();
    auto res = m_Cli->Get("/tickets");
    if (res && res->status == 200)
    {
        nlohmann::json data = nlohmann::json::parse(res->body);
        m_Tickets = data;
    }
    else
    {
        std::cout << "Canot get Ticket list\n";
    }

    //config SSE
    //Timeout
    m_SSEClient->on_event("TicketTimeout", [this](const httplib::sse::SSEMessage& msg) {
        m_BookedID = -1;
        //error call
        if(m_Callback)
            m_Callback("Booking Expired");
        });

    //Other booking
    m_SSEClient->on_event("TickedBooked", [this](const httplib::sse::SSEMessage& msg) {
        nlohmann::json response = nlohmann::json::parse(msg.data);
        uint32_t bookedID = response["tickedID"].get<uint32_t>();
        std::cout << "BoockedTicket" << bookedID << std::endl;
        //m_Tickets.erase(bookedID);
        auto it = m_Tickets.find(bookedID);
        if (it != m_Tickets.end())
        {
            it->second.Count--;
        }
        });

    //New ticket Aveable
    m_SSEClient->on_event("TickedAvailable", [this](const httplib::sse::SSEMessage& msg)
        {
            std::cout << "New ticket aveable\n";
            std::cout << msg.data;
            nlohmann::json response = nlohmann::json::parse(msg.data);
            uint32_t bookedID = response["tickedID"].get<uint32_t>();
            auto it = m_Tickets.find(bookedID);
            if (it != m_Tickets.end())
            {
                it->second.Count++;
            }
            /*
            TicketData emptyTicket;
            emptyTicket.Cost = 170;
            emptyTicket.ID = response["tickedID"].get<uint32_t>();
            m_Tickets[emptyTicket] = emptyTicket;
            */
        });
    //----
    m_SSEClient->start_async();
}

BiletomatCli::~BiletomatCli()
{
    m_SSEClient->stop();
}

bool BiletomatCli::BookTicket(uint32_t ticketID)
{
    //call server
    ClientRequest request;
    request.ClientID = m_MachineID;
    request.TicketID = ticketID;
    nlohmann::json requestJson = request;

    auto r = m_Cli->Post("/book", requestJson.dump(), "application/json");
    if (r && r->status == 200)
    {
        //std::cout << "Raw response: " << r->body << std::endl;
        nlohmann::json response = nlohmann::json::parse(r->body);
        uint32_t responseID = response.at("id").get<uint32_t>();
        //std::cout << "Server replied: " << responseID << "\n";
        if (responseID == ticketID)
        {
            m_BookedID = ticketID;
            return true;
        }
        else
        {
            if (m_Callback)
                m_Callback("Ticket Booking Error");
            m_BookedID = -1;
            return false;
        }
    }
    if (m_Callback)
        m_Callback("Ticket Booking Error");
    m_BookedID = -1;
    return false;
}

bool BiletomatCli::ConfirmOrder(const std::string& name, const std::string& surname)
{
    if (m_BookedID == -1)
    {
        return false;
    }

    BookingData request;
    request.ClientID = m_MachineID;
    request.TicketID = m_BookedID;
    request.Name = name;
    request.Surname = surname;

    nlohmann::json requestJson = request;
    auto r = m_Cli->Post("/confirm", requestJson.dump(), "application/json");
    if (r && r->status == 200)
    {
        ServerResponse response = nlohmann::json::parse(r->body);
        if (response.Status == 0)
        {
            m_BookedID = -1;
            return true;
        }
        else
        {
            if (m_Callback)
                m_Callback(response.Massage);
            return false;
        }
    }

    if (m_Callback)
        m_Callback("Ticket Confirmation Error");
    m_BookedID = -1;
    return false;
}

void BiletomatCli::CancelOrder()
{
    ClientRequest request;
    request.ClientID = m_MachineID;
    request.TicketID = m_BookedID;

    nlohmann::json requestJson = request;
    auto r = m_Cli->Post("/cancel", requestJson.dump(), "application/json");

    m_BookedID = -1;
}

uint32_t BiletomatCli::GetTicketPrice(uint32_t ticketTypeID) const
{
    auto it = m_Tickets.find(ticketTypeID);
    return it != m_Tickets.end() ? it->second.Cost : -1;
}

void BiletomatCli::SetCallback(const ErrorCallbackFn& callback)
{
    m_Callback = callback;
}
