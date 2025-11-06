#include "stdafx.h"
#include "keyboard.h" 
#include "Network.h"
#include "console.h" 

// keyboard.cpp 


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
, _input_length(0) 
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
        if (KeyState[i].state == KEY_TAPP) {
            if (i == ENTER) {
                // _input_buffer[_input_length] = '\0';
                if (_input_length > 0)
					Network::Manager::GetInstance().SendMsg(string(_input_buffer));
				memset(_input_buffer, 0, BUFFER_SIZE); 
                _input_length = 0;
            }
            else if (i == BACKSPACE) {
                if (_input_length > 0) {
                    _input_length--;
                    _input_buffer[_input_length] = '\0';
                }
            }
            else if (_input_length < BUFFER_SIZE - 1) {
                BYTE keyboardState[256];
                BOOL GetKeyboardStateOutput = GetKeyboardState(keyboardState);
                WCHAR wChar[4];
                int vk = KeyData[i];
                int result = ToUnicode(vk, 0, keyboardState, wChar, 4, 0);
                if (result == 1) {
                    char ascii = (char)wChar[0];
                    if (ascii >= 32 && ascii < 127) {
                        _input_buffer[_input_length++] = ascii;
                        _input_buffer[_input_length] = '\0';
                    }
                }
            }
        }
    }
}

void Input::Manager::Render() noexcept 
{
    short console_height = Console::Manager::GetInstance().GetHeight();
    if (KeyState[ENTER].state == KEY_TAPP) {
        Console::Manager::GetInstance().clear_line(console_height - 1);
    }
    string input_line = "> " + string(_input_buffer);  
	Console::Manager::GetInstance().draw_line(console_height - 1, input_line.c_str());
}

