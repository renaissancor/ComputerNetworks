#pragma once




class IOCompletionPort
{
public:
	IOCompletionPort(void) = default; 
	~IOCompletionPort(void) {
		WSACleanup();
	}

	bool InitSocket() {
		WSADATA wsaData;
		int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (wsaResult != 0) {
			fprintf_s(stderr, "Function WSAStartup() failed with error: %d\n", WSAGetLastError());
			return false;
		}

		_socketListen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (_socketListen == INVALID_SOCKET) {
			fprintf_s(stderr, "Function WSASocket() failed with error: %d\n", WSAGetLastError());
			return false;
		}

		fprintf_s(stdout, "Server Listen Socket initialization success.\n");
		return true;
	}

	bool BindAndListen(int nBindPort) const
	{
		SOCKADDR_IN serverAddr = {};
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(nBindPort);
		serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		int bindResult = bind(_socketListen, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
		if (bindResult != 0) {
			fprintf_s(stderr, "Function bind() failed with error: %d\n", WSAGetLastError());
			return false;
		}

		int listenResult = listen(_socketListen, 5);
		if (listenResult != 0) {
			fprintf_s(stderr, "Function listen() failed with error: %d\n", WSAGetLastError());
			return false;
		}

		fprintf_s(stdout, "Server Bind and Listen success.\n");
		return true; 
	}

	bool StartIOCPServer(const int maxClientCount) {
		CreateClient(maxClientCount); 
		_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, MAX_WORKER_THREAD);
		if (_iocpHandle == NULL) {
			fprintf_s(stderr, "Function CreateIoCompletionPort() failed with error: %d\n", GetLastError());
			return false;
		}

		// Create Worker Thread and Accept Thread Here ... 
		bool createWorkerThreadsResult = CreateWorkerThreads();
		if (!createWorkerThreadsResult) return false;
		bool createAcceptThreadResult = CreateAcceptThread();
		if (!createAcceptThreadResult) return false;

		fprintf_s(stdout, "IOCP Server Start Success.\n");
		return true; 
	}

	void DestroyThreads() {
		_isWorkerThreadRunning = false; 
		CloseHandle(_iocpHandle); 

		for (std::thread& thread : _arrIOWorkerThreads) {
			if (thread.joinable())
				thread.join();
		}

		_isAcceptThreadRunning = false;
		closesocket(_socketListen);
		if (_acceptThread.joinable())
			_acceptThread.join();
			
	}

	virtual void OnClientConnect(const UINT32 clientIndex) {}
	virtual void OnClientDiscconnect(const UINT32 clientIndex) {}
	virtual void OnClientReceive(const UINT32 clientIndex,
		const UINT32 size, char* pData) {
	}

private:
	void CreateClient(const int maxClientCount) {
		for (int i = 0; i < maxClientCount; ++i) {
			_arrClientInfos.emplace_back();
		}
	}

	bool CreateWorkerThreads() {
		unsigned int thread_id = 0; 
		for (int i = 0; i < MAX_WORKER_THREAD; ++i) {
			_arrIOWorkerThreads.emplace_back([this]() {
				WorkerThreadProc();
				});
		}
		fprintf_s(stdout, "IOCP Worker Threads Creation Success - Total Worker Threads: %d\n", MAX_WORKER_THREAD);
		return true; 
	}

	bool CreateAcceptThread() {
		_acceptThread = std::thread([this]() {
			AcceptThreadProc();
			});
		fprintf_s(stdout, "IOCP Accept Thread Creation Success.\n");
		return true;
	}

	ClientInfo* GetEmptyClientInfo() {
		for (ClientInfo& clientInfo : _arrClientInfos) {
			if (clientInfo._socketClient == INVALID_SOCKET) {
				return &clientInfo;
			}
		}
		return nullptr; 
	}

	bool BindIOCP(ClientInfo* pClientInfo) const {
		HANDLE hIOCP = CreateIoCompletionPort(
			(HANDLE)pClientInfo->_socketClient, 
			_iocpHandle, (ULONG_PTR)pClientInfo, 0);
		if (hIOCP == NULL || _iocpHandle != hIOCP) {
			fprintf_s(stderr, "Function CreateIoCompletionPort() failed with error: %d\n", GetLastError());
			return false;
		}
		return true; 
	}

	bool BindRecv(ClientInfo* pClientInfo) {
		DWORD dwFlag = 0;
		DWORD dwBytesRecv = 0; 
		pClientInfo->_recvOverlappedEx._wsaBuffer.len = MAX_SOCKBUF; 
		pClientInfo->_recvOverlappedEx._wsaBuffer.buf = pClientInfo->_recvBuffer;
		pClientInfo->_recvOverlappedEx._ioOperation = IOOperation::RECV; 

		int wsaRecvResult = WSARecv(
			pClientInfo->_socketClient,
			&pClientInfo->_recvOverlappedEx._wsaBuffer,
			1,
			&dwBytesRecv,
			&dwFlag,
			(LPWSAOVERLAPPED) &pClientInfo->_recvOverlappedEx._wsaOverlapped,
			NULL);

		if (wsaRecvResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			fprintf_s(stderr, "Function WSARecv() failed with error: %d\n", WSAGetLastError());
			return false;
		}
		return true;
	}

	bool SendData(ClientInfo* pClientInfo, char* pData, int len)
	{
		DWORD dwRecvBytes = 0; 
		CopyMemory(pClientInfo->_sendBuffer, pData, len); 

		pClientInfo->_sendOverlappedEx._wsaBuffer.len = len;
		pClientInfo->_sendOverlappedEx._wsaBuffer.buf = pClientInfo->_sendBuffer;
		pClientInfo->_sendOverlappedEx._ioOperation = IOOperation::SEND; 
		
		int wsaSendResult = WSASend(
			pClientInfo->_socketClient,
			&pClientInfo->_sendOverlappedEx._wsaBuffer,
			1,
			&dwRecvBytes,
			0,
			(LPWSAOVERLAPPED)&pClientInfo->_sendOverlappedEx._wsaOverlapped,
			NULL);

		if (wsaSendResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			fprintf_s(stderr, "Function WSASend() failed with error: %d\n", WSAGetLastError());
			return false;
		}
		return true; 
	}

	void WorkerThreadProc() {
		ClientInfo* pClientInfo = nullptr; 
		BOOL fCallSuccess = TRUE; 
		DWORD IOByteSize = 0; 
		LPOVERLAPPED lpOverlapped = nullptr; 

		while(_isWorkerThreadRunning) {
			fCallSuccess = GetQueuedCompletionStatus(
				_iocpHandle,
				&IOByteSize,
				(PULONG_PTR)&pClientInfo,
				&lpOverlapped,
				INFINITE); 

			if(fCallSuccess == TRUE && IOByteSize == 0 && lpOverlapped == nullptr) {
				// Client Disconnect Handling Here ... 
				_isWorkerThreadRunning = false;
				continue; 
			}
			if (lpOverlapped == nullptr) {
				continue; 
			}
			if (fCallSuccess == FALSE || IOByteSize == 0) {
				// Error Handling Here ... 
				fprintf_s(stderr, "[ERROR] : CLIENT %4lld Disconnected.\n", pClientInfo->_socketClient);
				CloseSocketClient(pClientInfo);
				continue; 
			}

			OverlappedEx* pOverlappedEx = (OverlappedEx*)lpOverlapped; 

			if (pOverlappedEx->_ioOperation == IOOperation::RECV) {
				pClientInfo->_recvBuffer[IOByteSize] = '\0'; 
				fprintf_s(stdout, "[RECV] %4d Bytes : %s\n", pClientInfo->_socketClient, pClientInfo->_recvBuffer);
				// Echo Back
				SendData(pClientInfo, pClientInfo->_recvBuffer, IOByteSize); 
				BindRecv(pClientInfo); 
			}
			else if (pOverlappedEx->_ioOperation == IOOperation::SEND) {
				// Send Completion Handling Here ... 
				fprintf_s(stdout, "[SEND] %4d Bytes : %s\n", IOByteSize, pClientInfo->_sendBuffer);
			}
			else {
				fprintf_s(stderr, "[ERROR] : CLIENT %4lld Unknown IO Operation.\n", pClientInfo->_socketClient);
			}
		}
	}

	void AcceptThreadProc() {
		SOCKADDR_IN clientAddr = {}; 
		int clientAddrLen = sizeof(SOCKADDR_IN);

		while(_isAcceptThreadRunning) {
			ClientInfo* pClientInfo = GetEmptyClientInfo(); 
			if(pClientInfo == nullptr) {
				fprintf_s(stderr, "[ERROR] No Free ClientInfo Available.\n"); 
				return; 
			}

			pClientInfo->_socketClient = accept(_socketListen, (SOCKADDR*)&clientAddr, &clientAddrLen);
			if (pClientInfo->_socketClient == INVALID_SOCKET) continue; 

			bool bindIOCPResult = BindIOCP(pClientInfo); 
			if (!bindIOCPResult) {
				closesocket(pClientInfo->_socketClient);
				pClientInfo->_socketClient = INVALID_SOCKET; 
				continue; 
			}

			bool bindRecvResult = BindRecv(pClientInfo); 
			if (!bindRecvResult) return; 

			char clientIP[32] = {}; 
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP)); 
			fprintf_s(stdout, "[INFO] Client Connected - IP: %s, PORT: %d, SOCKET: %d\n",
				clientIP,
				ntohs(clientAddr.sin_port),
				pClientInfo->_socketClient);
			++_countClient; 
		}
	}

	void CloseSocketClient(ClientInfo* pClientInfo, bool enforce = false) {
		struct linger linger_now = {0, 0}; 
		if (enforce) linger_now.l_onoff = 1; 

		shutdown(pClientInfo->_socketClient, SD_BOTH); 
		setsockopt(pClientInfo->_socketClient, SOL_SOCKET, SO_LINGER, (char*)&linger_now, sizeof(linger_now));
		closesocket(pClientInfo->_socketClient);
		pClientInfo->_socketClient = INVALID_SOCKET;
	}

private: 
	std::vector<ClientInfo> _arrClientInfos; 
	SOCKET _socketListen = INVALID_SOCKET; 
	int _countClient = 0; 

	std::vector<std::thread> _arrIOWorkerThreads;
	std::thread _acceptThread; 

	HANDLE _iocpHandle = INVALID_HANDLE_VALUE;
	volatile bool _isWorkerThreadRunning = true; 
	volatile bool _isAcceptThreadRunning = true;
	char _socketBuffer[MAX_SOCKBUF];
}; 
