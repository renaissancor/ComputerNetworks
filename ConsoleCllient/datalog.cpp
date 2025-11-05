#include "stdafx.h"	
#include "datalog.h" 
// datalog.cpp

Log::Manager Log::Manager::instance; 

Log::Manager::Manager()
	: _hFile(INVALID_HANDLE_VALUE), _hFileMapping(INVALID_HANDLE_VALUE), 
	_cs_log(), _ptr_now(nullptr), _line_used(0), _line_next(0) 
{}

Log::Manager::~Manager()
{} 

bool Log::Manager::Initiate() noexcept {
	InitializeCriticalSection(&_cs_log);
	_hFile = CreateFile(
		LOG_FILE_NAME,
		GENERIC_READ | GENERIC_WRITE,		// We want to read and write to the file. 
		FILE_SHARE_READ,					// Allow other processes to read the file while we have it open. 
		NULL,								// Default security attributes. 
		OPEN_ALWAYS,						// If the file exists, open it. If it does not exist, create a new file. 
		FILE_ATTRIBUTE_NORMAL,				// Normal file attributes.
		NULL								// No template file. 
	);

	if (_hFile == INVALID_HANDLE_VALUE) {
		DeleteCriticalSection(&_cs_log);
		return false;
	}

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
		DeleteCriticalSection(&_cs_log); 
		return false;
	}

	_ptr_now = static_cast<wchar_t*>(MapViewOfFile(
		_hFileMapping, 
		FILE_MAP_ALL_ACCESS,	// Read/write access. 
		0,						// High-order DWORD of the file offset where mapping begins.
		0,						// Low-order DWORD of the file offset where mapping begins.
		0						// Map the entire file. 
	));

	if (_ptr_now == NULL) {
		CloseHandle(_hFileMapping);
		CloseHandle(_hFile);
		DeleteCriticalSection(&_cs_log);
		return false;
	}

	return true; 
}

void Log::Manager::Shutdown() noexcept {
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

void Log::Manager::Record(const wchar_t* format, ...) noexcept {
	wchar_t temp_buffer[ONE_LINE_MAX_LENGTH]; 
	va_list args;
	va_start(args, format);
	vswprintf_s(temp_buffer, ONE_LINE_MAX_LENGTH, format, args);
	va_end(args); 

	EnterCriticalSection(&_cs_log);

	size_t target_index = _line_next % LINES_PER_PAGE;
	wchar_t* target_ptr = _ptr_now + (target_index * ONE_LINE_MAX_LENGTH);
	wmemcpy(target_ptr, temp_buffer, ONE_LINE_MAX_LENGTH);

	_line_next++;
	if (_line_used < LINES_PER_PAGE) {
		_line_used++; 
	}

	LeaveCriticalSection(&_cs_log);
}

void Log::Manager::WriteToConsole(
		wchar_t** console_buffer, short console_width, short console_height) noexcept {
	EnterCriticalSection(&_cs_log);
	
	for (short i = 0; i < console_height; ++i) {
		size_t relative_index_from_end = console_height - 1 - i;

		if (relative_index_from_end >= _line_used) {
			wmemset(console_buffer[i], L' ', console_width);
			console_buffer[i][console_width - 1] = L'\0';
			continue;
		}

		size_t log_index = (_line_next - 1 - relative_index_from_end + LINES_PER_PAGE) % LINES_PER_PAGE; 
		const wchar_t* log_ptr = _ptr_now + (log_index * ONE_LINE_MAX_LENGTH); 

		size_t log_len = wcsnlen(log_ptr, ONE_LINE_MAX_LENGTH);
		size_t copy_len = min(log_len, (size_t)console_width - 1);

		wmemcpy(console_buffer[i], log_ptr, copy_len);
		if (copy_len < (size_t)console_width - 1) {
			wmemset(console_buffer[i] + copy_len, L' ', console_width - 1 - copy_len);
		}
		console_buffer[i][console_width - 1] = L'\0';
	}
	LeaveCriticalSection(&_cs_log);
}