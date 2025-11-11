#include "stdafx.h"
#include "Network.h"
#include "console.h"
#include "Logger.h"

// Network.cpp 

Network::Manager Network::Manager::instance; 

Network::Manager::Manager()
	: _wsa(), _hSocket(INVALID_SOCKET), _server_addr()
{
}

Network::Manager::~Manager()
{
}

bool Network::Manager::Initiate() noexcept {
	// Initialize Network Code Here 
	if (::WSAStartup(MAKEWORD(2, 2), &_wsa) != 0) {
		return false; 
	}
	_hSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	if (_hSocket == INVALID_SOCKET) {
		::WSACleanup(); 
		return false; 
	}
	_server_addr.sin_family = AF_INET;
	_server_addr.sin_port = htons(SERVER_PORT); 
	::InetPtonW(AF_INET, SERVER_IP, &_server_addr.sin_addr); 
	int connect_result = ::connect(_hSocket, (SOCKADDR*)&_server_addr, sizeof(_server_addr)); 
	if (connect_result == SOCKET_ERROR) {
		::closesocket(_hSocket); 
		::WSACleanup(); 
		return false; 
	}
	int opt = 1;
	::setsockopt(_hSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(opt)); 

	u_long mode = 1; // Non-Blocking Mode
	if (::ioctlsocket(_hSocket, FIONBIO, &mode) == SOCKET_ERROR) {
		::closesocket(_hSocket);
		::WSACleanup();
		return false;
	}

	return true; 
}

void Network::Manager::Shutdown() noexcept {
	// Shutdown Network Code Here 
	if (_hSocket != INVALID_SOCKET) {
		::closesocket(_hSocket);
		_hSocket = INVALID_SOCKET;
	}
	::WSACleanup(); 
}

void Network::Manager::SendMsg(const string& msg) noexcept { 
	// Send Message to Server by Network code 
	// For now, just store the log in pending logs 
	char buffer[4096]; 
	int msg_length = static_cast<int>(msg.length());
	memcpy_s(buffer, sizeof(buffer), msg.c_str(), msg_length); 
	::send(_hSocket, buffer, msg_length, 0); 
} 

void Network::Manager::SendMsg(const char* format, ...) noexcept { 
	char buffer[SEND_BUFFER_SIZE];
	va_list args; 
	va_start(args, format); 
	int msg_length = vsnprintf_s(buffer, SEND_BUFFER_SIZE, _TRUNCATE, format, args);
	va_end(args); 

	if (msg_length <= 0 || msg_length >= SEND_BUFFER_SIZE) {
		// 메시지 포맷 실패 또는 버퍼 오버플로우 처리
		Logger::Manager::GetInstance().Log("[WARNING] SendMsg format failed or too long: %s", format);
		return;
	}

	const char* data_ptr = buffer;
	int total_bytes_to_send = msg_length;
	int total_bytes_sent = 0;

	while (total_bytes_sent < total_bytes_to_send) {
		int bytes_sent = ::send(
			_hSocket,
			data_ptr + total_bytes_sent,
			total_bytes_to_send - total_bytes_sent,
			0
		);
		if (bytes_sent == SOCKET_ERROR) {
			Logger::Manager::GetInstance().Log("[ERROR] Send failed: %d", WSAGetLastError());
			return;
		}
		total_bytes_sent += bytes_sent;
	}
}

void Network::Manager::SendDummyData() noexcept { 
	// For Test 

	// const char* dummy_message = "Hello from Chat Client!"; // How to make abouat 4KB Dummy data? 
	const int msg_length = 146000; // 65536 * 1000; // 64MB // 1460 * 1000; // About 14.6 KB 
	char buffer[msg_length];
	memset(buffer, 'A', sizeof(buffer) - 1); 

	const char* data_ptr = buffer;
	int total_bytes_to_send = msg_length;
	int total_bytes_sent = 0;
	
	while (total_bytes_sent < total_bytes_to_send) {
		int bytes_sent = ::send(
			_hSocket,
			data_ptr + total_bytes_sent,
			total_bytes_to_send - total_bytes_sent,
			0
		);
		if (bytes_sent == SOCKET_ERROR) {
			Logger::Manager::GetInstance().Log("[ERROR] Send failed: %d", WSAGetLastError());
			return;
		}
		total_bytes_sent += bytes_sent;
	}
}


void Network::Manager::RecvMsg(const Log& log) noexcept { 

}

void Network::Manager::Update() noexcept { 
	char _recv_buffer[4096]; 
	int bytes_received = 0; 
	int _recv_buffer_length = sizeof(_recv_buffer); 
	int recv_status = SOCKET_ERROR; 
	do {
		bytes_received = ::recv(_hSocket, _recv_buffer, sizeof(_recv_buffer), 0);
		if (bytes_received > 0) {
			Logger::Manager::GetInstance().Log("[Server] %.*s", bytes_received, _recv_buffer);
		}
		else if (bytes_received == 0) { // Connection Closed Gracefully 
			break;
		}
		else { // SOCKET_ERROR 
			int error = WSAGetLastError();
			// Non blocking socket did not receive data 
			if (error == WSAEWOULDBLOCK) {
				break;
			}
			else {
				// Network Error 
				Logger::Manager::GetInstance().Log("[ERROR] Recv failed: %d", error);
				break;
			}
		}
	} while (bytes_received > 0);
}

void Network::Manager::Render() noexcept { 
	Console::Manager& render_manager = Console::Manager::GetInstance(); 
	/*
	size_t data_count = _data.size(); 
	size_t start_index = data_count >= 20 ? data_count - 20 : 0;
	for (size_t i = 0; i < 20; i++) {
		size_t data_index = start_index + i; 
		if (data_index < data_count) {
			const string& str = _data[data_index];
			render_manager.draw_line(static_cast<short>(i + 3), str.c_str()); 
		}
		else {
			render_manager.draw_line(static_cast<short>(i + 3), ""); 
		}
	}
	*/
}
