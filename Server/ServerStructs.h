#pragma once
#include <string>
#include <chrono>

struct BookingEntry
{
    uint32_t TicketID;
    std::string ClientID;
    std::chrono::steady_clock::time_point BookedTime;
};

struct History
{
    uint32_t TicketID;
    std::string ClientID;
    std::string Name;
    std::string Surname;
};