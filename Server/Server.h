#pragma once

#include <cstdint>
#include <string>
#include <chrono>
#include "../Common/StructDefiicions.h"

//enum class TicketState { AVAILABLE, BOOKED, SOLD };

struct BookingEntry
{
    uint32_t TicketID;
    std::string ClientID;
    std::chrono::steady_clock::time_point BookedTime;
};

//struct Ticket
//{
//    uint32_t Cost = 10;
//    TicketState State = TicketState::AVAILABLE;
//    std::string Owner;
//
//    BookingEntry Booking;
//};

struct History
{
    //TicketData Ticket;
    uint32_t TicketID;
    std::string ClientID;
    std::string Name;
    std::string Surname;
};


