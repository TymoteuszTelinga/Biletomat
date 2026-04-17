
#include "TextField.h"

#include <rlgl.h>
#include <raymath.h>

TextField::TextField(Vector2& mousePosition, bool& mouseState, const std::string& placeholder, int fontSize)
	:m_MousePosition(mousePosition), bMousePressed(mouseState), m_Placeholder(placeholder), m_FontSize(fontSize)
{
}

void TextField::Update()
{
	if (!bActive)
		return;

	int character = GetCharPressed();
	while (character > 0)
	{
		char ch = static_cast<char>(character);

		if (isalpha(ch))
		{
			m_Value += ch;
		}
		character = GetCharPressed();
	}

	if (IsKeyPressed(KEY_BACKSPACE) && !m_Value.empty())
	{
		m_Value.pop_back();
	}
}

void TextField::Draw(Rectangle rect)
{
	DrawRectangleRec(rect, GRAY);
	Color BorderColor = bActive ? GREEN : BLACK;
	DrawRectangleLinesEx(rect, 1.5, BorderColor);

	const char* textToDisplay = m_Value.empty() ? m_Placeholder.c_str() : m_Value.c_str();
	int textX = rect.x + 10;
	int textY = rect.y + rect.height / 2 - m_FontSize / 2;

	Color textColor = m_Value.empty() ? LIGHTGRAY : BLACK;
	DrawText(textToDisplay, textX, textY, m_FontSize ,textColor);

	Matrix trasform = rlGetMatrixTransform();
	Matrix inv = MatrixInvert(trasform);

	Vector2 localMouse = {
		inv.m0 * m_MousePosition.x + inv.m4 * m_MousePosition.y + inv.m12,
		inv.m1 * m_MousePosition.x + inv.m5 * m_MousePosition.y + inv.m13
	};

	if (bMousePressed)
	{
		bActive = CheckCollisionPointRec(localMouse, rect);
	}
}
