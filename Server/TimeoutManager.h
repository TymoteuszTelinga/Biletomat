#pragma once

#include <thread>
#include <vector>;
#include <mutex>
#include <functional>

#include "ServerStructs.h"

class TimeoutManager
{
public:
	using ExpiredCallbackFn = std::function<void(const std::vector<std::string>& expired)>;

	TimeoutManager(std::unordered_map<std::string, BookingEntry>& bookings, std::mutex& mutex);
	~TimeoutManager();

	void Start(uint32_t interval = 10);
	void Stop();

	void SetCallback(const ExpiredCallbackFn& callback);
	void SetTimeout(float timeoutDuration);

private:
	void CheckExpiredBooking();

private:

	std::unordered_map<std::string, BookingEntry>& m_Bookings;
	std::mutex& m_BookigMutex;

	ExpiredCallbackFn m_Callback;

	std::thread m_Worker;
	bool m_Running = false;
	float m_TimeoutDuration = 60.f;
};