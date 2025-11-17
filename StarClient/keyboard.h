#pragma once
// keyboard.h 

namespace Keyboard {
    enum KEY {
        // 0~9, -, =
        _1, _2, _3, _4, _5, _6, _7, _8, _9, _0, MINUS, PLUS,
        // QWERTY... [ ] 
        Q, W, E, R, T, Y, U, I, O, P, LBRACKET, RBRACKET, BACKSLASH,
        // ASDF... ; '
        A, S, D, F, G, H, J, K, L, SEMICOLON, APOSTROPHE,
        // ZXCV... , . /
        Z, X, C, V, B, N, M, COMMA, PERIOD, SLASH,
        // Special/Modifier Keys
        TILDE, // ~
        TAB, BACKSPACE,
        SHIFT, CTRL, ALT,
        DEL,
        // System Keys
        ENTER, ESCAPE, SPACE,
        // Direction Keys
        LEFT, UP, RIGHT, DOWN,
        // Mouse Buttons (Left/Right)
        LBTN, RBTN,
        KEY_COUNT
    };

    enum KEY_STATE {
        KEY_IDLE,		// 0 : No Press 
        KEY_TAPP,		// 1 : Pressed Moment 
        KEY_HOLD,		// 2 : During Press
        KEY_FREE		// 3 : Released Moment 
    };

    struct KeyInfo {
        KEY_STATE state;
        bool pressed; // Pressed in Previous Tick 
    };

    class Manager {
    private: // data 
        KeyInfo KeyState[KEY_COUNT]; 
        static const int KeyData[KEY_COUNT];
	private: // functions 
        static Manager instance;
        Manager();
        ~Manager();
        Manager(Manager const&) = delete;
        Manager const& operator=(Manager const&) = delete;
    public:
        inline static Manager& GetInstance() noexcept { return instance; }
        inline const KEY_STATE GetKeyState(KEY key) 
            const noexcept { return KeyState[key].state; }
        inline const bool IsKeyTapp(KEY key) const noexcept { return KeyState[key].state == KEY_TAPP; }
        inline const bool IsKeyHold(KEY key) const noexcept { return KeyState[key].state == KEY_HOLD; }
        inline const bool IsKeyFree(KEY key) const noexcept { return KeyState[key].state == KEY_FREE; }

        void Update() noexcept;

    }; // class Manager 
} // namespace Keyboard 