#pragma once

#include "stdafx.h"
#include "console.h" 
// console.cpp 

namespace Console {
	constexpr const SHORT DEFAULT_WIDTH = 80;
	constexpr const SHORT DEFAULT_HEIGHT = 25;

	class Manager {
	private:
		static Manager instance;
		Manager(Manager const&) = delete;
		Manager const& operator=(Manager const&) = delete;
	private:
		alignas(64)
		SHORT _width;
		SHORT _height;
		char* _buffer_write;
		char* _buffer_print;
		HANDLE _hConsole;
		CRITICAL_SECTION _cs;

	private:
		Manager();
		~Manager();

	private:
		void CreateBuffer(short width, short height) noexcept;
		void DeleteBuffer() noexcept;

		inline void InternalMoveCursor(short x, short y) const noexcept {
			COORD coord = { x, y };
			SetConsoleCursorPosition(_hConsole, coord);
		}

	public:
		inline static Manager& GetInstance() noexcept { return instance; }
		inline const short GetHeight() const noexcept { return _height; } 

		inline void draw(short y, short x, char ch) noexcept {
			if (y < 0 || y >= _height || x < 0 || x >= _width) return;
			EnterCriticalSection(&_cs);
			_buffer_write[y * _width + x] = ch;
			LeaveCriticalSection(&_cs);
		}
		inline void draw_line(short y, const char* str) noexcept {
			if (y < 0 || y >= _height) return;
			size_t index = static_cast<size_t>(y) * _width; 
			size_t len = min(strlen(str), static_cast<size_t>(_width));
			// memcpy(&_buffer_write[index], str, len);
			memcpy_s(&_buffer_write[index], _width, str, len); 
			if (len < _width) memset(&_buffer_write[index + len], ' ', _width - len);
		}

		inline void draw_line(short y, short x, const char* str) noexcept {
			if (y < 0 || y >= _height || x < 0 || x >= _width) return;
			size_t index = static_cast<size_t>(y) * _width + x;
			size_t len = min(strlen(str), static_cast<size_t>(_width - x));
			// memcpy(&_buffer_write[index], str, len);
			memcpy_s(&_buffer_write[index], _width - x, str, len); 
			if (len < static_cast<size_t>(_width - x))
				memset(&_buffer_write[index + len], ' ', _width - x - len);
		}

		inline void clear_line(short y) noexcept {
			if (y < 0 || y >= _height) return;
			size_t index = static_cast<size_t>(y) * _width;
			memset(&_buffer_write[index], ' ', _width);
		}
		

		void Resize(short width, short height) noexcept;
		void Render() noexcept;

		bool Initiate();
		void Shutdown();

	};
} // namespace Console 
