#pragma once

constexpr const unsigned int MAX_SOCKBUF = 1024;
constexpr const unsigned int MAX_WORKER_THREAD = 4;

enum class IOOperation {
	RECV,
	SEND
};

struct OverlappedEx
{
	WSAOVERLAPPED _wsaOverlapped;
	SOCKET _socketClient;
	WSABUF _wsaBuffer;
	IOOperation _ioOperation;
};

struct ClientInfo
{
	SOCKET _socketClient;
	OverlappedEx _recvOverlappedEx;
	OverlappedEx _sendOverlappedEx;

	char _recvBuffer[MAX_SOCKBUF];
	char _sendBuffer[MAX_SOCKBUF];

	ClientInfo() :
		_socketClient(INVALID_SOCKET)
	{
		// ZeroDeviceMemory(&_recvOverlappedEx, sizeof(_recvOverlappedEx));
		// ZeroDeviceMemory(&_sendOverlappedEx, sizeof(_sendOverlappedEx));
		ZeroMemory(&_recvOverlappedEx, sizeof(_recvOverlappedEx));
		ZeroMemory(&_sendOverlappedEx, sizeof(_sendOverlappedEx));
	}
};