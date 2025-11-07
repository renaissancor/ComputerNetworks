#include "stdafx.h"
#include "Logger.h" 
#include "console.h"

// Logger.cpp 

Logger::Manager Logger::Manager::instance; 

Logger::Manager::Manager()
	: _hFile(INVALID_HANDLE_VALUE), _hFileMapping(INVALID_HANDLE_VALUE), 
	_ptr_now(nullptr), _cs_log() 
{}

Logger::Manager::~Manager()
{}

bool Logger::Manager::Initiate() noexcept {
	DWORD pid = GetCurrentProcessId(); 
	wchar_t log_file_name[128];
	swprintf_s(log_file_name, _countof(log_file_name),
		L"%s_%lu.txt",
		LOG_FILE_NAME,
		pid);
	// unsigned short port = Network::Manager::GetInstance().GetLocalPort(); 
	// swprintf_s(log_file_name, _countof(log_file_name), 
	//            L"%s_%hu.txt", LOG_FILE_NAME, port);

	_hFile = CreateFile(
		log_file_name,
		GENERIC_READ | GENERIC_WRITE,		// We want to read and write to the file. 
		FILE_SHARE_READ,					// Allow other processes to read the file while we have it open. 
		NULL,								// Default security attributes. 
		OPEN_ALWAYS,						// If the file exists, open it. If it does not exist, create a new file. 
		FILE_ATTRIBUTE_NORMAL,				// Normal file attributes.
		NULL								// No template file. 
	);

	if (_hFile == INVALID_HANDLE_VALUE) return false; 
	LARGE_INTEGER file_size = { 0 };
	file_size.QuadPart = FILE_SIZE_BYTES;
	SetFilePointerEx(_hFile, file_size, NULL, FILE_BEGIN);
	SetEndOfFile(_hFile);

	_hFileMapping = CreateFileMapping(
		_hFile,
		NULL,					// Default security attributes.
		PAGE_READWRITE,			// Read/write access.
		file_size.HighPart,		// High-order DWORD of the maximum size of the file mapping object.
		file_size.LowPart,		// Low-order DWORD of the maximum size of the file mapping object. 
		NULL					// No name for the mapping object. 
	);

	if (_hFileMapping == NULL) {
		CloseHandle(_hFile);
		return false;
	}

	_ptr_now = static_cast<char*>(MapViewOfFile(
		_hFileMapping,
		FILE_MAP_ALL_ACCESS,	// Read/write access. 
		0,						// High-order DWORD of the file offset where mapping begins.
		0,						// Low-order DWORD of the file offset where mapping begins.
		0						// Map the entire file. 
	));

	if (_ptr_now == nullptr) {
		CloseHandle(_hFileMapping);
		CloseHandle(_hFile);
		return false;
	}

	InitializeCriticalSection(&_cs_log); 

	return true;
}

void Logger::Manager::Shutdown() noexcept {
	if (_ptr_now != nullptr) {
		UnmapViewOfFile(_ptr_now);
		_ptr_now = nullptr;
	}

	if (_hFileMapping != INVALID_HANDLE_VALUE) {
		CloseHandle(_hFileMapping);
		_hFileMapping = INVALID_HANDLE_VALUE;
	}

	if (_hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(_hFile);
		_hFile = INVALID_HANDLE_VALUE;
	}

	DeleteCriticalSection(&_cs_log); 
}

void Logger::Manager::Log(const char* format, ...) noexcept {
	char temp_buffer[ONE_LINE_MAX_LENGTH];
	va_list args;
	va_start(args, format);
	int len = vsnprintf(temp_buffer, ONE_LINE_MAX_LENGTH, format, args);
	va_end(args);

	if (len < 0 || len >= ONE_LINE_MAX_LENGTH) {
		return;
	}

	size_t message_length = (size_t)len;

	EnterCriticalSection(&_cs_log); 

	size_t current_offset = _write_offset;
	size_t total_write_length = message_length + 1; 

	if (current_offset + total_write_length >= FILE_SIZE_BYTES) {
		current_offset = 0; 
	}

	_line_start_ptrs.push_back(current_offset);

	char* ptr_target = _ptr_now + current_offset;
	memcpy_s(ptr_target, FILE_SIZE_BYTES - current_offset, temp_buffer, message_length);
	ptr_target[message_length] = '\n';

	_write_offset = current_offset + total_write_length;
	LeaveCriticalSection(&_cs_log); 
}


// Logger.cpp - Logger::Manager::Render()
void Logger::Manager::Render() noexcept {

	std::vector<size_t> offsets_to_render;
	size_t current_write_offset;
	size_t total_size = FILE_SIZE_BYTES;

	EnterCriticalSection(&_cs_log);

	// 현재 쓰기 오프셋을 스냅샷으로 확보
	current_write_offset = _write_offset;

	const size_t display_count = Console::Manager::GetInstance().GetHeight() - 4;
	size_t vec_size = _line_start_ptrs.size();

	size_t start_index = (vec_size > display_count) ? (vec_size - display_count) : 0;

	offsets_to_render.reserve(display_count);
	for (size_t i = start_index; i < vec_size; ++i) {
		offsets_to_render.push_back(static_cast<size_t>(_line_start_ptrs[i]));
	}

	LeaveCriticalSection(&_cs_log);

	char* ptr_now = _ptr_now;
	int lines_printed = 0;

	for (size_t i = 0; i < offsets_to_render.size(); ++i) {
		size_t current_offset = offsets_to_render[i];

		size_t next_offset;

		if (i < offsets_to_render.size() - 1) {
			next_offset = offsets_to_render[i + 1];
		}
		else {
			next_offset = current_write_offset;
		}

		size_t log_length;
		if (next_offset > current_offset) {
			log_length = next_offset - current_offset;
		}
		else {
			log_length = (total_size - current_offset) + next_offset;
		}

		size_t message_length = (log_length > 0) ? (log_length - 1) : 0;

		char* ptr_log_start = ptr_now + current_offset;

		Console::Manager::GetInstance().draw_line_cut( 
			static_cast<short>(lines_printed + 1),
			ptr_log_start,
			message_length 
		);
		lines_printed++;
	}
}