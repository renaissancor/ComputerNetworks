#pragma once

// console.h 

namespace Console {
	constexpr const SHORT DEFAULT_WIDTH = 80;
	constexpr const SHORT DEFAULT_HEIGHT = 25;

	class Manager {
	private:
		Manager();
		~Manager();
		Manager(Manager const&) = delete;
		Manager const& operator=(Manager const&) = delete;
	private:
		static Manager instance;
		SHORT _width = DEFAULT_WIDTH;
		SHORT _height = DEFAULT_HEIGHT;
		CHAR_INFO* _bufferWrite = nullptr;
		CHAR_INFO* _bufferPrint = nullptr;
		HANDLE _hConsole = INVALID_HANDLE_VALUE;
	private:
		void CreateBuffer(short width, short height) noexcept;
		void DeleteBuffer() noexcept;
		/*
		inline void InternalMoveCursor(short x, short y) const noexcept {
			COORD coord = { x, y };
			SetConsoleCursorPosition(_hConsole, coord);
		}
		*/
	public:
		inline static Manager& GetInstance() noexcept { return instance; }
		inline void Draw(short y, short x, char ch, WORD color = 0x07) noexcept {
			if (y < 0 || y >= _height || x < 0 || x >= _width) return;
			CHAR_INFO& cell = _bufferWrite[y * _width + x];
			cell.Char.AsciiChar = ch;
			cell.Attributes = color;
		}
		void DrawLine(short y, const char* str) noexcept;
		void DrawLine(short y, short x, const char* str) noexcept;
		void ResizeConsole(short width, short height) noexcept;
		void Display() noexcept;

		bool Initialize() noexcept;
		void Shutdown() noexcept;

	};
} // namespace Console 