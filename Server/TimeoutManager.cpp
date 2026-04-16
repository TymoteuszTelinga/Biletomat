
#include "TimeoutManager.h"
#include <chrono>
#include <iostream>

TimeoutManager::TimeoutManager(std::unordered_map<std::string, BookingEntry>& bookings, std::mutex& mutex)
	: m_Bookings(bookings), m_BookigMutex(mutex)
{

}

TimeoutManager::~TimeoutManager()
{
	Stop();
}

void TimeoutManager::Start(uint32_t interval)
{
	m_Running = true;
	m_Worker = std::thread([this, interval]() 
		{
			while (m_Running)
			{
				CheckExpiredBooking();

				for (size_t i = 0; i < interval; i++)
				{
					if (!m_Running)
						break;
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
			}
		});
}

void TimeoutManager::Stop()
{
	m_Running = false;
	m_Worker.join();
}

void TimeoutManager::SetCallback(const ExpiredCallbackFn& callback)
{
	m_Callback = callback;
}

void TimeoutManager::SetTimeout(float timeoutDuration)
{
	m_TimeoutDuration = timeoutDuration;
}

void TimeoutManager::CheckExpiredBooking()
{
	std::cout << "CheckRun\n";
	std::vector<std::string> expiredBookings;
	auto now = std::chrono::steady_clock::now();
	{
		std::lock_guard<std::mutex> lock(m_BookigMutex);
		for (const auto& [id, entry] : m_Bookings)
		{
			uint32_t elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - entry.BookedTime).count();
			if (elapsed >= m_TimeoutDuration)
			{
				expiredBookings.push_back(id);
			}
		}
	}
	if (!expiredBookings.empty())
	{
		m_Callback(expiredBookings);
	}
}
