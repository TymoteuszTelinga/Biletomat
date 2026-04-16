#pragma once

#include <raylib.h>
#include <string>

class TextField
{
public:
	TextField(Vector2& m_MousePosition, bool& bMousePressed, const std::string& placeholder, int fontSize);
    ~TextField() {};

    void Update();
    void Draw(Rectangle rect);

    void Clear() { m_Value.clear(); }
    std::string GetValue() const { return m_Value; };

private:
    Vector2& m_MousePosition;
    bool& bMousePressed;

    std::string m_Value;
    std::string m_Placeholder;
    int m_MaxLen = 64;
    bool bActive = false;
    int m_FontSize = 10;
    int m_CursorTick = 0;   // blink counter
};