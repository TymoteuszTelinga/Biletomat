#pragma once

#include <httplib/httplib.h>
#include <nlohmann/json.hpp>

#include <vector>

class Broadcaster
{
public:

	void AddClient(const std::string& clientID, httplib::DataSink& sink);

	void RemoveClient(const std::string& clientID);

	void Broadcast(const std::string& eventName, const nlohmann::json& data);
	void SentToClient(const std::string& clientID, const std::string& eventName, const nlohmann::json& data);

private:

	bool Send(httplib::DataSink* sink, const std::string& eventName, const nlohmann::json& data);

	struct Client
	{
		std::string ClientID;
		httplib::DataSink* Sink;
		bool bActive = true;
	};

	std::mutex m_Mutex;
	std::vector<Client> m_Clients;
};