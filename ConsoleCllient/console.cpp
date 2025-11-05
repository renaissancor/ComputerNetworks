#include "stdafx.h"
#include "console.h" 
#include "datalog.h" 
// console.cpp 

Console::Manager Console::Manager::instance; 

Console::Manager::Manager() : _hConsole(INVALID_HANDLE_VALUE), 
_cs(), _input_buffer(), 
_width(DEFAULT_WIDTH), _height(DEFAULT_HEIGHT), 
_buffer_write(nullptr), _buffer_print(nullptr)
{}

Console::Manager::~Manager()
{}

// Private Methods 

void Console::Manager::CreateBuffer(short width, short height) noexcept {
	_width = width, _height = height;
	_buffer_write = new wchar_t* [_height];
	for (short i = 0; i < _height; ++i) {
		_buffer_write[i] = new wchar_t[_width];
		wmemset(_buffer_write[i], L' ', _width);
		_buffer_write[i][_width - 1] = L'\0';
	}
	_buffer_print = new wchar_t* [_height];
	for (short i = 0; i < _height; ++i) {
		_buffer_print[i] = new wchar_t[_width];
		wmemset(_buffer_print[i], L' ', _width);
		_buffer_print[i][_width - 1] = L'\0';
	}
}

void Console::Manager::DeleteBuffer() noexcept {
	if (_buffer_write == nullptr) return;
	for (short i = 0; i < _height; ++i)
		delete[] _buffer_write[i];
	delete[] _buffer_write;
	_buffer_write = nullptr;
	for (short i = 0; i < _height; ++i)
		delete[] _buffer_print[i];
	delete[] _buffer_print; 
}

// Public Methods 

bool Console::Manager::Initiate() {
	_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (_hConsole == INVALID_HANDLE_VALUE) return false;
	InitializeCriticalSection(&_cs);
	CONSOLE_CURSOR_INFO cci = { sizeof(cci), FALSE }; // invisible cursor 
	SetConsoleCursorInfo(_hConsole, &cci);
	CreateBuffer(_width, _height);
	return true;
}

void Console::Manager::Shutdown() {
	DeleteCriticalSection(&_cs);
	DeleteBuffer();
	_hConsole = INVALID_HANDLE_VALUE;
}

void Console::Manager::Resize(short width, short height) noexcept {
	DeleteBuffer();
	CreateBuffer(width, height);
	CONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };
	GetConsoleScreenBufferInfo(_hConsole, &csbi);

	COORD BufferSize = { (SHORT)width, (SHORT)height };
	SetConsoleScreenBufferSize(_hConsole, BufferSize);

	SMALL_RECT rect = { 0, 0, (SHORT)(width - 1), (SHORT)(height - 1) };
	SetConsoleWindowInfo(_hConsole, TRUE, &rect);

	HWND console = GetConsoleWindow();
	if (console == NULL) return;
	RECT rc;
	GetWindowRect(console, &rc);

	int win_w = rc.right - rc.left;
	int win_h = rc.bottom - rc.top;

	int x = (GetSystemMetrics(SM_CXSCREEN) - win_w) / 2;
	int y = 0;

	SetWindowPos(console, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void Console::Manager::Update() noexcept {
	EnterCriticalSection(&_cs); 
	short log_height = _height - 1; 

	Log::Manager::GetInstance().WriteToConsole(_buffer_write, _width, log_height);
	short input_row = _height - 1; 

	const wchar_t* input_msg = _input_buffer.c_str();
	size_t input_len = _input_buffer.length();
	size_t copy_len = min(input_len, (size_t)_width - 1);

	wmemcpy(_buffer_write[input_row], input_msg, copy_len);
	if (copy_len < (size_t)_width - 1) {
		wmemset(_buffer_write[input_row] + copy_len, L' ', _width - 1 - copy_len);
	}
	_buffer_write[input_row][_width - 1] = L'\0';

	LeaveCriticalSection(&_cs);
}

void Console::Manager::Render() noexcept {
	EnterCriticalSection(&_cs);
	for (short i = 0; i < _height; ++i)
		wmemcpy(_buffer_print[i], _buffer_write[i], _width);
	LeaveCriticalSection(&_cs);
	for (short i = 0; i < _height; ++i) {
		InternalMoveCursor(0, i);
		wprintf(L"%s", _buffer_print[i]);
	}
}

