#pragma once

// Logger.h 

namespace Logger {
	using std::vector; 
	constexpr const wchar_t* LOG_FILE_NAME = L"chat_client_log"; 
	constexpr const size_t PAGE_SIZE_BYTES = 65536; // 64 KB 
	constexpr const size_t FILE_SIZE_BYTES = PAGE_SIZE_BYTES * 100; // 6400 KB, 6.4 MB  
	constexpr const size_t ONE_LINE_MAX_LENGTH = 4096; // char per line 
	class Manager {
	private:
		static Manager instance;
		Manager(Manager const&) = delete;
		Manager const& operator=(Manager const&) = delete;
	private:
		HANDLE _hFile;
		HANDLE _hFileMapping; 
		CRITICAL_SECTION _cs_log; 

		char* _ptr_now = nullptr; 
		volatile size_t _write_offset = 0;
		vector<uintptr_t> _line_start_ptrs; 

	private:
		Manager();
		~Manager();
	public:
		inline static Manager& GetInstance() noexcept { return instance; }

		bool Initiate() noexcept; 
		void Shutdown() noexcept; 

		void Log(const char* format, ...) noexcept; 
		void Render() noexcept; 
		
	};
} // namespace Logger