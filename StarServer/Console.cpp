#include "stdafx.h"
#include "console.h"

// Console.cpp 

Console::Manager Console::Manager::instance;

Console::Manager::Manager()
{
}

Console::Manager::~Manager()
{
}

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
	_bufferWrite = new CHAR_INFO[size];
	_bufferPrint = new CHAR_INFO[size];
	CHAR_INFO defaultCell = { ' ', 0x07 };
	for (size_t i = 0; i < size; ++i) {
		_bufferWrite[i] = defaultCell;
		_bufferPrint[i] = defaultCell;
	}
}

void Console::Manager::DeleteBuffer() noexcept {
	delete[] _bufferWrite;
	delete[] _bufferPrint;
	_bufferWrite = nullptr;
	_bufferPrint = nullptr;
}

void Console::Manager::DrawLine(short y, const char* str) noexcept {
	if (y < 0 || y >= _height) return;
	DrawLine(y, 0, str);
}

void Console::Manager::DrawLine(short y, short x, const char* str) noexcept {
	if (y < 0 || y >= _height || x < 0 || x >= _width) return;

	size_t index = static_cast<size_t>(y) * _width + x;
	size_t maxLen = static_cast<size_t>(_width - x);
	size_t len = strlen(str);

	for (size_t i = 0; i < len && i < maxLen; ++i) {
		CHAR_INFO& cell = _bufferWrite[index + i];
		cell.Char.AsciiChar = str[i];
		cell.Attributes = 0x07;
	}
}

void Console::Manager::ResizeConsole(short width, short height) noexcept {
	DeleteBuffer();
	CreateBuffer(width, height);
}

void Console::Manager::Display() noexcept {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(_hConsole, &csbi)) {
		short newW = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		short newH = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		if (newW != _width || newH != _height) {
			ResizeConsole(newW, newH);
		}
	}

	COORD bufferSize = { _width, _height };
	COORD bufferCoord = { 0, 0 };
	SMALL_RECT writeRegion = {
		0, 0,
		_width - 1, _height - 1
	};

	WriteConsoleOutput(
		_hConsole,
		_bufferWrite,
		bufferSize,
		bufferCoord,
		&writeRegion
	);

	CHAR_INFO defaultCell = { ' ', 0x07 };
	size_t total = static_cast<size_t>(_width) * _height;
	for (size_t i = 0; i < total; ++i) {
		_bufferWrite[i] = defaultCell;
	}
}

bool Console::Manager::Initialize() noexcept {
	_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (_hConsole == INVALID_HANDLE_VALUE) return false;

	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(_hConsole, &cursorInfo);
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(_hConsole, &cursorInfo);

	CreateBuffer(_width, _height);
	return true;
}

void Console::Manager::Shutdown() noexcept {
	DeleteBuffer();
	_hConsole = INVALID_HANDLE_VALUE;
}
