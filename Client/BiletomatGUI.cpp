
#include "BiletomatGUI.h"

#include <rlgl.h>
#include <raymath.h>
#include <format>

BiletomatGui::BiletomatGui()
    :m_NameForm(m_MousePosition, bMousePressed, "Name", 30),
    m_SurnameForm(m_MousePosition, bMousePressed, "Surname", 30)
{
    m_BiletomatCLI.SetCallback([this](const std::string& msg) {OnError(msg);});
    m_CashRegister.LoadState("cash.txt");
	InitWindow(c_Width, c_Height, "Biletomat");
}

BiletomatGui::~BiletomatGui()
{
	CloseWindow();
}

void BiletomatGui::Run()
{
	while (!WindowShouldClose())
	{
		m_MousePosition = GetMousePosition();
        bMousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

		OnUpdate();

		OnRender();
	}
}

void BiletomatGui::OnUpdate()
{
    if (bDrawPopup)
    {
        auto curentTime = std::chrono::steady_clock::now();
        float elapsedSec = std::chrono::duration_cast<std::chrono::microseconds>(curentTime - m_PopupTime).count() / 1000000.f;
        if (elapsedSec >= c_PopupDuration)
        {
            bDrawPopup = false;
        }
    }

    m_NameForm.Update();
    m_SurnameForm.Update();
}

void BiletomatGui::OnRender()
{
	BeginDrawing();
	ClearBackground({ 245, 244, 241, 255 });

    switch (m_State)
    {
    case MachineState::Selection:
        DrawSelectionScreen();
        break;
    case MachineState::PersonalInformation:
        DrawPersonalInformationScreen();
        break;
    case MachineState::Payment:
        DrawPaymentScreen();
        break;
    default:
        break;
    }
    if (bDrawPopup)
    {
        DrawPopupMessage(m_PopupMsg);
    }

	EndDrawing();
}

bool BiletomatGui::Button(const Rectangle& rect, const std::string& text, uint32_t fontSize)
{
    int textWidth = MeasureText(text.c_str(), fontSize);
    int textHeight = fontSize;

    int textX = (int)(rect.x + (rect.width - textWidth) / 2);
    int textY = (int)(rect.y + (rect.height - textHeight) / 2);

    DrawRectangleRounded(rect, 0.2f, 10, LIGHTGRAY);
    DrawText(text.c_str(), textX, textY, fontSize, BLACK);

    Matrix trasform = rlGetMatrixTransform();
    Matrix inv = MatrixInvert(trasform);

    Vector2 localMouse = {
        inv.m0 * m_MousePosition.x + inv.m4 * m_MousePosition.y + inv.m12,
        inv.m1 * m_MousePosition.x + inv.m5 * m_MousePosition.y + inv.m13
    };

    if (bMousePressed)
    {
        return CheckCollisionPointRec(localMouse, rect);
    }
    return false;
}

bool BiletomatGui::DrawTicket(const TicketData& ticket, uint32_t positionX, uint32_t positionY)
{
    const int padding = 10;
    const int fontSize = 30;
    const int halfWidth = 150;

    bool bAveable = ticket.Count > 0;

    Rectangle rect(positionX - halfWidth, positionY, halfWidth * 2, fontSize + 2 * padding);
    DrawRectangleRounded(rect, 0.2f, 10, LIGHTGRAY);

    Color TextColor = bAveable ? BLACK : GRAY;

    DrawText(ticket.Name.c_str(), positionX-halfWidth+padding, positionY+padding, fontSize, TextColor);

    uint32_t zlote = ticket.Cost / 100;
    uint32_t grosze = ticket.Cost % 100;
    std::string price = std::format("{:01d}.{:02d} zl", zlote, grosze);
    int textLength = MeasureText(price.c_str(), fontSize);
    int priceX = positionX + halfWidth - padding - textLength;
    DrawText(price.c_str(), priceX, positionY + padding, fontSize, TextColor);

    if (!bAveable)
    {
        return false;
    }

    //colision check
    Matrix trasform = rlGetMatrixTransform();
    Matrix inv = MatrixInvert(trasform);

    Vector2 localMouse = {
        inv.m0 * m_MousePosition.x + inv.m4 * m_MousePosition.y + inv.m12,
        inv.m1 * m_MousePosition.x + inv.m5 * m_MousePosition.y + inv.m13
    };

    if (bMousePressed)
    {
        return CheckCollisionPointRec(localMouse, rect);
    }
    return false;
}

void BiletomatGui::DrawPopupMessage(const std::string& message)
{
    const int screenWidth = c_Width;
    const int FontSize = 50;
    const int screenHeight = FontSize + 20;
    const int positionX = 0;
    const int positionY = (c_Height - screenHeight) / 2;

    DrawRectangle(positionX, positionY, screenWidth, screenHeight, GRAY);

    int textWidth = MeasureText(message.c_str(), FontSize);
    DrawText(message.c_str(), (c_Width-textWidth)/2, (c_Height-FontSize)/2, FontSize, DARKPURPLE);
}

void BiletomatGui::DrawSelectionScreen()
{
    int column = c_Width / 2;
    uint32_t counter = 0;

    auto ticketsData = m_BiletomatCLI.GetAvailableTickets();
    for (const auto& [id, ticketData] : ticketsData)
    {
        if (DrawTicket(ticketData, column, 100 + counter * 80))
        {
            if (m_BiletomatCLI.BookTicket(id))
            {
                m_Price = m_BiletomatCLI.GetTicketPrice(id);
                m_State = MachineState::PersonalInformation;
            }
        }
        counter++;
    }
}

void BiletomatGui::DrawPersonalInformationScreen()
{
    const int screenWidth = 400;
    const int screenHeight = 100 + 60 * 3 + 10 * 2;
    const int positionX = (c_Width - screenWidth) / 2;
    const int positionY = (c_Height - screenHeight) / 2;
    DrawRectangle(positionX, positionY, screenWidth, screenHeight, GRAY);

    //DrawRectangle(positionX, positionY, screenWidth, screenHeight, GRAY);
    rlPushMatrix();
    rlTranslatef(positionX, positionY, 0);

    m_NameForm.Draw({ 50, 50, 300, 60 });
    m_SurnameForm.Draw({ 50, 50 + 70, 300, 60 });
    /*m_NameForm.Draw();
    m_SurnameForm.Draw();*/
    if (Button({ 50, 50 + 2 * 60 + 2 * 10, 145,60 }, "Continue", 30))
    {
        if (!m_NameForm.GetValue().empty() && !m_SurnameForm.GetValue().empty())
        {
            //m_BookingData.Name = m_NameForm.GetValue();
            //m_BookingData.Surname = m_SurnameForm.GetValue();
            m_State = MachineState::Payment;
        }
    }

    if (Button({ 50 + 145 + 10, 50 + 2 * 60 + 2 * 10, 145,60 }, "Cancel", 30))
    {
        m_BiletomatCLI.CancelOrder();
        ClearState();
    }

    rlPopMatrix();
}

void BiletomatGui::DrawPaymentScreen()
{
    const int screenWidth = 490 + 50 - 150;
    const int screenHeight = 400 + 50 - 100;
    const int positionX = (c_Width - screenWidth) / 2;
    const int positionY = (c_Height - screenHeight) / 2;
    DrawRectangle(positionX, positionY, screenWidth, screenHeight, GRAY);

    rlPushMatrix();
    rlTranslatef(positionX, positionY+50, 0);

    uint32_t remning = m_Price - m_Total;
    uint32_t zl = remning / 100;
    uint32_t gr = remning % 100;
    std::string price = std::format("Wrzuc\t{:d}.{:02d} zl", zl, gr);
    DrawText(price.c_str(), 50, 10, 30, BLACK);

    if (Button({ 50, 50, 90, 60 }, "5 zl", 30))
    {
        AddCoin(Nominal::zl5);
    }
    if (Button({ 150, 50, 90, 60 }, "2 zl", 30))
    {
        AddCoin(Nominal::zl2);
    }
    if (Button({ 250, 50, 90, 60 }, "1 zl", 30))
    {
        AddCoin(Nominal::zl1);
    }

    if (Button({ 50, 120, 90, 60 }, "50 gr", 30))
    {
        AddCoin(Nominal::gr50);
    }
    if (Button({ 150, 120, 90, 60 }, "20 gr", 30))
    {
        AddCoin(Nominal::gr20);
    }
    if (Button({ 250, 120, 90, 60 }, "10 gr", 30))
    {
        AddCoin(Nominal::gr10);
    }

    if (Button({ 50, 190, 290, 60 }, "Cancel", 30))
    {
        m_BiletomatCLI.CancelOrder();
        ClearState();
    }

    rlPopMatrix();
}

void BiletomatGui::ClearState()
{
    m_Total = 0;
    for (uint32_t i = 0; i < 6; i++)
    {
        m_Coins[i] = 0;
    }
    m_State = MachineState::Selection;

    m_NameForm.Clear();
    m_SurnameForm.Clear();
}

void BiletomatGui::OnError(const std::string& msg)
{
    std::cout << "Error: " << msg << std::endl;
    m_PopupMsg = msg;
    EnablePopup();
    ClearState();
}

void BiletomatGui::EnablePopup()
{
    bDrawPopup = true;
    m_PopupTime = std::chrono::steady_clock::now();
}

void BiletomatGui::AddCoin(Nominal nom)
{
    m_Total += CashRegister::CoinMultiplayer(nom);
    m_Coins[nom]++;

    if (m_Total >= m_Price)
    {
        bool bSucess = m_CashRegister.GetChange(m_Total - m_Price, m_Coins);
        if (bSucess)
        {
            m_BiletomatCLI.ConfirmOrder(m_NameForm.GetValue(), m_SurnameForm.GetValue());
            ClearState();

            m_PopupMsg = "Printing ticket";
            EnablePopup();
        }
        else
        {
            m_BiletomatCLI.CancelOrder();
            OnError("Canot give a change");
        }
    }
}
