#pragma once

#include <raylib.h>
#include <chrono>

#include "Biletomat.h"
#include "CashRegister.h"
#include "TextField.h"

class BiletomatGui
{
public:
	BiletomatGui();
	~BiletomatGui();

	void Run();
	void OnUpdate();
	void OnRender();

private:
	bool Button(const Rectangle& rect, const std::string& text, uint32_t fontSize);
	bool DrawTicket(const TicketData& ticket, uint32_t positionX, uint32_t positionY);
	void DrawPopupMessage(const std::string& message);

	void DrawSelectionScreen();
	void DrawPersonalInformationScreen();
	void DrawPaymentScreen();

	void ClearState();
	void AddCoin(enum Nominal nom);

	void OnError(const std::string& msg);

	void EnablePopup();

private:
	BiletomatCli m_BiletomatCLI;

	enum class MachineState
	{
		Selection,
		PersonalInformation,
		Payment
	};
	MachineState m_State = MachineState::Selection;

	uint32_t m_Price = 1270;
	uint32_t m_Total = 0;
	uint32_t m_Coins[6] = { 0,0,0,0,0,0 };
	CashRegister m_CashRegister;

	TextField m_NameForm;
	TextField m_SurnameForm;

	Vector2 m_MousePosition;
	bool bMousePressed = false;
	const uint32_t c_Width = 720;
	const uint32_t c_Height = 720;

	bool bDrawPopup = false;
	std::string m_PopupMsg;
	std::chrono::steady_clock::time_point m_PopupTime;
	const float c_PopupDuration = 5.f;
};