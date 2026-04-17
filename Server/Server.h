#pragma once

#include <cstdint>
#include <string>
#include <chrono>
#include <httplib/httplib.h>

#include "../Common/StructDefiicions.h"

#include "ServerStructs.h"

#include "BroadcastHub.h"
#include "TimeoutManager.h"

class TicketServer
{
public:

    TicketServer();
    ~TicketServer();

    void LoadData(const std::filesystem::path& dataPath);
    void Run();
    void Stop();

private:

    void OnEvents(const httplib::Request& req, httplib::Response& res);
    void OnGetTickets(const httplib::Request& req, httplib::Response& res);
    void OnBook(const httplib::Request& req, httplib::Response& res);
    void OnCancel(const httplib::Request& req, httplib::Response& res);
    void OnConfirm(const httplib::Request& req, httplib::Response& res);

    void RemoveBooking(const std::string& clientID);

    void OnBookingsExpired(const std::vector<std::string>& expiredBookings);

private:

    httplib::Server m_Svr;
    Broadcaster m_Broadcaster;
    TimeoutManager m_TimeoutManager;

    std::unordered_map<uint32_t, TicketData> m_AvailableTickets;
    std::vector<History> m_SoldTickets;

    std::unordered_map<std::string, BookingEntry> m_BookedTickets;
    std::mutex m_BookedMutex;
};

