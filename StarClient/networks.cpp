#include "stdafx.h"
#include "networks.h" 
#include "gameplay.h"
#include "keyboard.h" 
#include "engine.h" 

Networks::Manager Networks::Manager::instance; 

Networks::Manager::Manager() 
	: _hSocket(INVALID_SOCKET)
	, _serverAddr()
{}

Networks::Manager::~Manager() 
{}

bool Networks::Manager::Initialize() noexcept {
	fwprintf_s(stdout, L"Enter Server IP Address : ");
	// fwscanf_s(stdin, L"%s", _ipAddr, INET_ADDRSTRLEN); 
	fwprintf_s(stdout, L"%s\n", _ipAddr); 
	wcscpy_s(_ipAddr, SERVER_IP); // For Convenience 
	_results._InetPtonWResult = ::InetPtonW(AF_INET, _ipAddr, &_serverAddr.sin_addr);
	if (_results._InetPtonWResult != 1) {
		fwprintf_s(stderr, L"Invalid IP Address Format!\n");
		return false;
	}
	else {
		wchar_t output_ip[INET_ADDRSTRLEN];
		::InetNtopW(AF_INET, &_serverAddr.sin_addr, output_ip, INET_ADDRSTRLEN);
		fwprintf_s(stdout, L"IP Address Set to %s\n", output_ip);
	}

	_results._WSAStartupResult = ::WSAStartup(MAKEWORD(2, 2), &_wsa);
	if (_results._WSAStartupResult != 0) {
		_results._WSALastErrorResult = WSAGetLastError();
		fwprintf_s(stderr, L"WSAStartup Failed! Error Code : %d\n", WSAGetLastError());
		return false;
	}

	_hSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_hSocket == INVALID_SOCKET) {
		_results._SocketCreateResult = WSAGetLastError();
		_results._WSALastErrorResult = WSAGetLastError();
		fwprintf_s(stderr, L"Socket Create Failed! Error Code : %d\n", WSAGetLastError());
		::WSACleanup();
		return false;
	}

	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = htons(SERVER_PORT);

	int opt = 1;
	_results._setsockoptResult = 
		::setsockopt(_hSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(opt));

	if (_results._setsockoptResult == SOCKET_ERROR) {
		_results._setsockoptResult = WSAGetLastError();
		_results._WSALastErrorResult = _results._setsockoptResult;
		fwprintf_s(stderr, L"Setsockopt Failed! Error Code : %d\n", _results._setsockoptResult);
		::closesocket(_hSocket);
		::WSACleanup();
		return false;
	}

	u_long mode = 1; // Non-Blocking Mode 
	_results._ioctlsocketResult = ::ioctlsocket(_hSocket, FIONBIO, &mode); 
	if (_results._ioctlsocketResult) {
		_results._ioctlsocketResult = WSAGetLastError();
		_results._WSALastErrorResult = _results._ioctlsocketResult;
		fwprintf_s(stderr, L"ioctlsocket Failed! Error Code : %d\n", WSAGetLastError());
		::closesocket(_hSocket);
		::WSACleanup();
		return false;
	}
	
	return true; 
}

bool Networks::Manager::Connect() noexcept {
	// Placeholder for future connection logic if needed
	_results._connectResult = ::connect(_hSocket, (SOCKADDR*)&_serverAddr, sizeof(_serverAddr)); 
	if (_results._connectResult == SOCKET_ERROR) {
		int wsaError = WSAGetLastError(); 
		if (wsaError == WSAEWOULDBLOCK) { // Non-Blocking Connect In Progress
			fwprintf_s(stdout, L"Connection attempt initiated (WSAEWOULDBLOCK).\n");
			_results._isConnectedToServer = 1;
			return true;
		}
		fwprintf_s(stderr, L"Socket Connect Failed! Error Code : %d\n", wsaError);
		::closesocket(_hSocket);
		_hSocket = INVALID_SOCKET; 
		_results._isConnectedToServer = 0;
		return false;
	}
	fwprintf_s(stdout, L"Connected to Server %s:%hu successfully.\n", _ipAddr, SERVER_PORT);
	return true;
}

void Networks::Manager::Shutdown() noexcept {
	if (_hSocket != INVALID_SOCKET) {
		::closesocket(_hSocket);
		_hSocket = INVALID_SOCKET;
	}
	_results._isConnectedToServer = 0;
	::WSACleanup();
}

void Networks::Manager::Send(const int x, const int y) noexcept {
	if (x == 0 && y == 0) return; 
	Protocol::Packet packet = { 
		3, 
		Gameplay::Manager::GetInstance().GetLocalPlayer()->id
		, static_cast<uint32_t>(x)
		, static_cast<uint32_t>(y) 
	}; // MOVE Packet 
	int bytesSent = ::send(_hSocket, 
		reinterpret_cast<const char*>(&packet), sizeof(Protocol::Packet), 0);
	_results._sendResult = bytesSent;
}

void Networks::Manager::Recv() noexcept {
	int bytesReceived = ::recv(_hSocket, _recvBuffer + _recvBufferOffset, 
		RECV_BUFFER_SIZE - _recvBufferOffset, 0);
	_results._recvResult = bytesReceived; 
	if (bytesReceived > 0) {
		_recvBufferOffset += bytesReceived; 
	} else if (bytesReceived == 0) {
		// Connection has been gracefully closed
		fwprintf_s(stdout, L"Connection closed by the server.\n");
		_results._isConnectedToServer = 0;
		Engine::GetInstance().StopRunning(); 
	}
	else {
		int wsaError = WSAGetLastError();
		if (wsaError == WSAEWOULDBLOCK) return; // Non Blocking Socket No Data Received 
		_results._WSALastErrorResult = wsaError;
		_results._isConnectedToServer = 0;
	}

	while (_recvBufferOffset >= sizeof(Protocol::Packet)) {
		Protocol::Packet* packet = reinterpret_cast<Protocol::Packet*>(_recvBuffer);
		int changeType; 
		switch (packet->type) {
		case 0: changeType = Gameplay::Change::LOGIN; break;
		case 1: changeType = Gameplay::Change::CREATE; break;
		case 2: changeType = Gameplay::Change::REMOVE; break;
		case 3: changeType = Gameplay::Change::MOVE; break;
		default: {
			_recvBufferOffset = 0; // Discard Invalid Data 
			return; }
		}

		Gameplay::Manager::GetInstance().EnqueueChange(
			changeType, packet->id, 
			static_cast<int>(packet->x), static_cast<int>(packet->y));

		int leftDataSize = _recvBufferOffset - sizeof(Protocol::Packet); 
		if (leftDataSize > 0) {
			memmove(_recvBuffer, _recvBuffer + sizeof(Protocol::Packet), leftDataSize);
		}
		_recvBufferOffset = leftDataSize;
	}
}