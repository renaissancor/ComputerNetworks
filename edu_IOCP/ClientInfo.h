#pragma once

class ClientInfo
{
public:
	ClientInfo() :
		_index(-1),
		_socket(INVALID_SOCKET)
	{
		ZeroMemory(&_recvOverlappedEx, sizeof(_recvOverlappedEx));
		ZeroMemory(_recvBuffer, sizeof(_recvBuffer));
	}

	void Init(const uint32_t index) {
		_index = index; 
	}

	inline uint32_t GetIndex() const { return _index; }
	inline bool IsConnected() const { return _socket != INVALID_SOCKET; }
	inline SOCKET GetSocket() const { return _socket; } 
	inline char* GetRecvBuffer() { return _recvBuffer; } 

	bool OnConnect(HANDLE iocpHandle, SOCKET socket_) {
		_socket = socket_;
		Clear(); 
		if(BindIOCompletionPort(iocpHandle) == false) {
			return false; 
		}
		return BindRecv(); 
	}

	void Close(bool isEnforced = false) {
		struct linger lngOption;
		if (isEnforced) lngOption.l_onoff = 0;
		
		shutdown(_socket, SD_BOTH);
		setsockopt(_socket, SOL_SOCKET, SO_LINGER, (char*)&lngOption, sizeof(lngOption));
		closesocket(_socket);
		_socket = INVALID_SOCKET; 
	}

	void Clear(){} 

	bool BindIOCompletionPort(HANDLE iocpHandle) {
		HANDLE hIOCP = CreateIoCompletionPort(
			(HANDLE)_socket,
			iocpHandle,
			(ULONG_PTR)(this),
			0);
		if (hIOCP == INVALID_HANDLE_VALUE) {
			fprintf_s(stderr, "CreateIoCompletionPort() failed. Error: %d\n", WSAGetLastError());
			return false;
		}
		return true;
	}

	bool BindRecv() {
		DWORD flags = 0;
		DWORD bytesReceived = 0;
		_recvOverlappedEx._ioOperation = IOOperation::RECV; 
		_recvOverlappedEx._wsaBuffer.len = MAX_SOCKBUF;
		_recvOverlappedEx._wsaBuffer.buf = _recvBuffer; 

		int recvResult = WSARecv(
			_socket,
			&_recvOverlappedEx._wsaBuffer,
			1,
			&bytesReceived,
			&flags,
			(LPWSAOVERLAPPED) & _recvOverlappedEx._wsaOverlapped,
			NULL);

		if (recvResult == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
			fprintf_s(stderr, "WSARecv() failed. Error: %d\n", WSAGetLastError());
			return false;
		}
		return true;
	}

	// Must be called in one thread 
	bool SendData(const UINT32 dataSize, const char* pData) {
		OverlappedEx* sendOverlappedEx = new OverlappedEx();
		ZeroMemory(sendOverlappedEx, sizeof(OverlappedEx)); 
		sendOverlappedEx->_ioOperation = IOOperation::SEND;
		sendOverlappedEx->_wsaBuffer.len = dataSize;
		sendOverlappedEx->_wsaBuffer.buf = new char[dataSize]; 
		// CopyMemory(sendOverlappedEx->_wsaBuffer.buf, pData, dataSize); 
		memcpy_s(sendOverlappedEx->_wsaBuffer.buf, dataSize, pData, dataSize); 

		DWORD recvBytes = 0; 
		int sendResult = WSASend(
			_socket,
			&sendOverlappedEx->_wsaBuffer,
			1,
			&recvBytes,
			0,
			(LPWSAOVERLAPPED)sendOverlappedEx,
			NULL);

		if (sendResult == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
			fprintf_s(stderr, "WSASend() failed. Error: %d\n", WSAGetLastError());
			return false;
		}

		return true; 
	}

	void SendCompleted(const uint32_t dataSize) const {
		// Nothing to do yet 
		fprintf_s(stdout, "[Client %d] Sent %d bytes.\n", _index, dataSize); 
	}

private: 
	uint32_t _index;
	SOCKET _socket; 
	OverlappedEx _recvOverlappedEx; 
	char _recvBuffer[MAX_SOCKBUF]; 
};

/*

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
*/