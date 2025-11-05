#include "stdafx.h"
#include "network.h" 
// network.cpp 

Network::Manager Network::Manager::instance; 

Network::Manager::Manager() : _wsa(), _hSocket(INVALID_SOCKET), _server_addr()
{}

Network::Manager::~Manager()
{}

bool Network::Manager::Initiate() noexcept {
	if (::WSAStartup(MAKEWORD(2, 2), &_wsa) != 0)
	{
		fprintf_s(stderr, "WSAStartup Failed! Error Code : %d\n", WSAGetLastError());
		return false;
	}
	_hSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_hSocket == INVALID_SOCKET) {
		fprintf_s(stderr, "Socket creation failed! Error Code : %d\n", WSAGetLastError());
		::WSACleanup();
		return false;
	}
	_server_addr.sin_family = AF_INET;
	_server_addr.sin_port = htons(SERVER_PORT);
	::InetPtonW(AF_INET, SERVER_IP, &_server_addr.sin_addr);
	int connect_result = ::connect(_hSocket, (SOCKADDR*)&_server_addr, sizeof(_server_addr));
	if (connect_result == SOCKET_ERROR) {
		fprintf_s(stderr, "Socket connection failed! Error Code : %d\n", WSAGetLastError());
		::closesocket(_hSocket);
		::WSACleanup();
		return false;
	}
	int opt = 1;
	::setsockopt(_hSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(opt));
	return true;
}

void Network::Manager::Shutdown() noexcept {
	if (_hSocket != INVALID_SOCKET) {
		::closesocket(_hSocket);
		_hSocket = INVALID_SOCKET;
	}
	::WSACleanup();
}

bool Network::Manager::SendPacket(const char* data, size_t length) noexcept {
	size_t total_sent = 0;
	while (total_sent < length) {
		int bytes_sent = ::send(_hSocket, data + total_sent, static_cast<int>(length - total_sent), 0);
		if (bytes_sent == SOCKET_ERROR) {
			return false;
		}
		total_sent += bytes_sent;
	}
	return true;
}
