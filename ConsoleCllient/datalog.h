#pragma once

// datalog.h 

using std::vector; 
using std::wstring; 

namespace Log {

	constexpr const WCHAR* LOG_FILE_NAME = L"chat_log.txt";
	constexpr const size_t PAGE_SIZE_BYTES = 65536; // 64 KB 
	constexpr const size_t FILE_SIZE_BYTES = PAGE_SIZE_BYTES * 100; // 6400 KB, 6.4 MB  
	constexpr const size_t ONE_LINE_MAX_LENGTH = 128; // whcar_t per line 
	constexpr const size_t LINES_PER_PAGE = // 65536 * 100 / (128 * 2) = 25600 lines maximum 
		FILE_SIZE_BYTES / (ONE_LINE_MAX_LENGTH * sizeof(wchar_t));
 
	class Manager {
	private:
		static Manager instance;
		Manager(Manager const&) = delete;
		Manager const& operator=(Manager const&) = delete;
		Manager();
		~Manager();

	private:
		HANDLE _hFile;
		HANDLE _hFileMapping; 
		CRITICAL_SECTION _cs_log; 
		wchar_t* _ptr_now = nullptr;

		size_t _line_used; 
		size_t _line_next; 

	public:
		inline static Manager& GetInstance() noexcept { return instance; } 
		inline bool Initiate() noexcept;
		inline void Shutdown() noexcept;

		void Record(const wchar_t* format, ...) noexcept; 
		void WriteToConsole(wchar_t** console_buffer, short console_width, short console_height) noexcept;
	};
} // namespace DataLog