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
		SHORT _width;
		SHORT _height;
		wchar_t** _buffer_write;
		wchar_t** _buffer_print; 
		HANDLE _hConsole;
		CRITICAL_SECTION _cs; 

		std::wstring _input_buffer; 
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
		
		void Resize(short width, short height) noexcept;
		void Update() noexcept; 
		void Render() noexcept; 

		bool Initiate(); 
		void Shutdown(); 


		void RefreshInputLine(const std::wstring& current_input = L"") {
			EnterCriticalSection(&_cs);

			InternalMoveCursor(0, _height - 1); // move to input line 
			wprintf(L"%s", std::wstring(_width, L' ').c_str()); // clear line 
			InternalMoveCursor(0, _height - 1); // move to input line 
			wprintf(L"> %s", current_input.c_str());

			LeaveCriticalSection(&_cs);
		}
	};
} // namespace Console 
