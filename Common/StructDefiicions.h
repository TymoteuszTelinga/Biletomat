#pragma once

#include <nlohmann/json.hpp>

struct BookingData
{
	uint32_t TicketID;
	std::string ClientID;

	std::string Name;
	std::string Surname;

	void Clear()
	{
		TicketID = -1;
		ClientID.clear();
		Name.clear();
		Surname.clear();
	}
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BookingData, TicketID, ClientID, Name, Surname);

struct ClientRequest
{
	uint32_t TicketID;
	std::string ClientID;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ClientRequest, TicketID, ClientID);

struct TicketData
{
	uint32_t Count;
	uint32_t Cost;
	std::string Name;
	//
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TicketData, Count, Cost, Name);

struct ServerResponse
{
	uint32_t Status;
	std::string Massage;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ServerResponse, Status, Massage);