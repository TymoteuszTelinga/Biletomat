#pragma once

#include <httplib/httplib.h>
#include <memory>
#include <unordered_map>
#include "../Common/StructDefiicions.h"


//mutiple tickest pool
//seats tickest at juve
//10 biletow normalnych
//5 biletow ulgowych
//struct TicketData
//{
//	uint32_t ID;
//	uint32_t Cost;
//	//
//};

class BiletomatCli
{
public:
	BiletomatCli();
	~BiletomatCli();

	bool BookTicket(uint32_t ticketID);
	bool ConfirmOrder(const std::string& name, const std::string& surname);
	void CancelOrder();

	const std::unordered_map<uint32_t, TicketData>& GetAvailableTickets() const { return m_Tickets; };
	uint32_t GetTicketPrice(uint32_t ticketTypeID) const;

	using ErrorCallbackFn = std::function<void(const std::string& msg)>;
	void SetCallback(const ErrorCallbackFn& callback);

private:
	std::string m_MachineID;
	std::unique_ptr<httplib::Client> m_Cli;
	std::unique_ptr<httplib::sse::SSEClient> m_SSEClient;

	uint32_t m_BookedID = -1;
	std::unordered_map<uint32_t, TicketData> m_Tickets;

	ErrorCallbackFn m_Callback;
};