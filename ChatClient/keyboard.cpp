#include "stdafx.h"
#include "keyboard.h" 

// keyboard.cpp 

using namespace Input; 

KeyInfo KeyState[KEY_COUNT];

int KeyData[KEY_COUNT] = {
    // 0~9, -, =
    '1','2','3','4','5','6','7','8','9','0', VK_OEM_MINUS, VK_OEM_PLUS,
	// QWERTY... [ ] backslash
    'Q','W','E','R','T','Y','U','I','O','P', VK_OEM_4, VK_OEM_6, VK_OEM_5,
    // ASDF... ; '
    'A','S','D','F','G','H','J','K','L', VK_OEM_1, VK_OEM_7,
    // ZXCV... , . /
    'Z','X','C','V','B','N','M', VK_OEM_COMMA, VK_OEM_PERIOD, VK_OEM_2,
    // Special/Modifier Keys
    VK_OEM_3,    // ~
    VK_TAB,      // TAB
    VK_BACK,     // BACKSPACE
    VK_SHIFT,    // SHIFT
    VK_CONTROL,  // CTRL
    VK_MENU,     // ALT
    VK_DELETE,   // DEL
    // System Keys
    VK_RETURN,   // ENTER
    VK_ESCAPE,   // ESCAPE
    VK_SPACE,    // SPACE
    // Direction Keys
    VK_LEFT,     // LEFT
    VK_UP,       // UP
    VK_RIGHT,    // RIGHT
    VK_DOWN,     // DOWN
    // Mouse Buttons (Left/Right)
    VK_LBUTTON,  // LBTN
    VK_RBUTTON   // RBTN
};

Input::Manager Input::Manager::instance;

Input::Manager::Manager() : _input_buffer() 
{
    for (size_t i = 0; i < KEY_COUNT; i++) {
        KeyState[i].state = KEY_IDLE;
        KeyState[i].pressed = false;
    }
}

Input::Manager::~Manager() 
{
}

void Input::Manager::Update() noexcept 
{
    for (size_t i = 0; i < KEY_COUNT; i++)
    {
        if (0x8000 & GetAsyncKeyState(KeyData[i]))
        {
            if (KeyState[i].pressed) KeyState[i].state = KEY_HOLD; // During Press
            else KeyState[i].state = KEY_TAPP; // Pressed Moment
            KeyState[i].pressed = true;
        }
        else {
            if (KeyState[i].pressed) KeyState[i].state = KEY_FREE; // Released Moment
            else KeyState[i].state = KEY_IDLE; // No Press
            KeyState[i].pressed = false;
        }
    }

    for (size_t i = 0; i < KEY_COUNT; i++) {

    }

}
