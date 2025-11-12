#pragma once

constexpr const uint32_t MAX_SOCKBUF = 256;
constexpr const uint32_t MAX_SOCKBUF_SEND = 4096;
constexpr const uint32_t MAX_WORKER_THREAD = 4;

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
