
#include "BroadcastHub.h"
#include <iostream>

void Broadcaster::AddClient(const std::string& clientID, httplib::DataSink& sink)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	m_Clients.push_back({ clientID, &sink });
    std::cout << "Client Connected\n";
	std::cout << "[SSE] clients " << m_Clients.size() << std::endl;
}

void Broadcaster::RemoveClient(const std::string& clientID)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Clients.erase(
        std::remove_if(m_Clients.begin(), m_Clients.end(),
            [&](const Client& c) { return c.ClientID == clientID; }),
        m_Clients.end()
    );
    std::cout << "Cient Disconected\n";
    std::cout << "[SSE] clients " << m_Clients.size() << std::endl;
}

void Broadcaster::Broadcast(const std::string& eventName, const nlohmann::json& data)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    for (Client& client : m_Clients)
    {
        if (client.bActive && client.Sink)
        {
            bool status = Send(client.Sink, eventName, data);
            client.bActive = status;
        }
        else
        {
            client.bActive = false;
        }
    }

    m_Clients.erase(
        std::remove_if(m_Clients.begin(), m_Clients.end(),
            [](const Client& c) { return !c.bActive; }),
        m_Clients.end()
    );
}

void Broadcaster::SentToClient(const std::string& clientID, const std::string& eventName, const nlohmann::json& data)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    for (Client& client : m_Clients)
    {
        if (client.ClientID == clientID && client.bActive) 
        {
            client.bActive = Send(client.Sink, eventName, data);
            break;
        }
    }
}

bool Broadcaster::Send(httplib::DataSink* sink, const std::string& eventName, const nlohmann::json& data)
{
    if (!sink->is_writable())
        return false;

    std::string msg = "event: " + eventName + "\n" +"data: " + data.dump() + "\n\n";
    std::cout << "raw msg: " << msg << std::endl;
    return sink->write(msg.c_str(), msg.size());
}
