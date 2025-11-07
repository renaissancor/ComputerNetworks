#pragma once
// Inputs.h 

enum KEY {
    // 0~9, -, =
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _0, MINUS, EQUAL,
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

extern KeyInfo KeyState[KEY_COUNT];

inline const KEY_STATE GetKeyState(KEY key) noexcept {
	return KeyState[key].state;
}

namespace Inputs {
	constexpr const size_t BUFFER_SIZE = 256; 

class Manager {
private:
	char _Inputs_buffer[BUFFER_SIZE]; 
    size_t _Inputs_length; 

private:
    static Manager instance;
    Manager(Manager const&) = delete;
	Manager const& operator=(Manager const&) = delete;

private: 
    Manager(); 
    ~Manager(); 

public:
    inline static Manager& GetInstance() noexcept { return instance; }
    void Update() noexcept; 
	void Render() noexcept; 
    
}; // class Manager 

} // namespace Inputs 