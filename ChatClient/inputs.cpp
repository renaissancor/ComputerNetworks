#include "stdafx.h"
#include "Inputs.h" 
#include "Network.h"
#include "console.h" 
#include "engine.h" 

// Inputss.cpp 


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

Inputs::Manager Inputs::Manager::instance;

Inputs::Manager::Manager() : _Inputs_buffer()
, _Inputs_length(0) 
{
    for (size_t i = 0; i < KEY_COUNT; i++) {
        KeyState[i].state = KEY_IDLE;
        KeyState[i].pressed = false;
    }
}

Inputs::Manager::~Manager() 
{
}

void Inputs::Manager::Update() noexcept 
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
                // _Inputs_buffer[_Inputs_length] = '\0'; 
                if (strcmp(_Inputs_buffer, "exit") == 0) {
                    Engine::GetInstance().StopRunning();
                    return; 
                }
                else  if (_Inputs_length > 0)
					Network::Manager::GetInstance().SendMsg(string(_Inputs_buffer));
				memset(_Inputs_buffer, 0, BUFFER_SIZE); 
                _Inputs_length = 0;
            }
            else if (i == BACKSPACE) {
                if (_Inputs_length > 0) {
                    _Inputs_length--;
                    _Inputs_buffer[_Inputs_length] = '\0';
                }
            }
            else if (_Inputs_length < BUFFER_SIZE - 1) {
                BYTE keyboardState[256];
                BOOL GetKeyboardStateOutput = GetKeyboardState(keyboardState);
                WCHAR wChar[4];
                int vk = KeyData[i];
                int result = ToUnicode(vk, 0, keyboardState, wChar, 4, 0);
                if (result == 1) {
                    char ascii = (char)wChar[0];
                    if (ascii >= 32 && ascii < 127) {
                        _Inputs_buffer[_Inputs_length++] = ascii;
                        _Inputs_buffer[_Inputs_length] = '\0';
                    }
                }
            }
        }
    }
}

void Inputs::Manager::Render() noexcept 
{
    short console_height = Console::Manager::GetInstance().GetHeight();
    if (KeyState[ENTER].state == KEY_TAPP) {
        Console::Manager::GetInstance().clear_line(console_height - 1);
    }
    string Inputs_line = "> " + string(_Inputs_buffer);  
	Console::Manager::GetInstance().draw_line(console_height - 1, Inputs_line.c_str());
}

