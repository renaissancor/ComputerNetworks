#include "stdafx.h"
#include "console.h" 
// console.cpp 

Console::Manager Console::Manager::instance;

Console::Manager::Manager() : _hConsole(INVALID_HANDLE_VALUE),
_cs(), 
_width(DEFAULT_WIDTH), _height(DEFAULT_HEIGHT),
_buffer_write(nullptr), _buffer_print(nullptr)
{
}

Console::Manager::~Manager()
{
}

// Private Methods 

void Console::Manager::CreateBuffer(short width, short height) noexcept {
	CONSOLE_SCREEN_BUFFER_INFO csbi; 
	if (GetConsoleScreenBufferInfo(_hConsole, &csbi)) {
		_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	}
	else {
		_width = width;
		_height = height;
	}

	size_t size = static_cast<size_t>(_width) * _height;
	_buffer_write = new char[size];
	_buffer_print = new char[size];
	memset(_buffer_write, ' ', size);
	memset(_buffer_print, ' ', size);
}

void Console::Manager::DeleteBuffer() noexcept {
	delete[] _buffer_write;
	delete[] _buffer_print;
	_buffer_write = nullptr;
	_buffer_print = nullptr;
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
}

void Console::Manager::Render() noexcept {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(_hConsole, &csbi)) {
		short newW = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		short newH = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		if (newW != _width || newH != _height) {
			Resize(newW, newH);
		}
	}

	size_t total = static_cast<size_t>(_width) * _height;
	EnterCriticalSection(&_cs);
	memcpy_s(_buffer_print, total, _buffer_write, total);
	LeaveCriticalSection(&_cs);

	COORD origin = { 0, 0 };
	DWORD written = 0;
	WriteConsoleOutputCharacterA(_hConsole, _buffer_print, (DWORD)total, origin, &written);
}

