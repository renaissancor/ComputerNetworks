#pragma once

#include "ClientInfo.h"

class IOCompletionPort
{
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

public:
	IOCompletionPort(void) {}
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

	bool StartServer(const uint32_t maxClientCount) {
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

	bool SendData(const uint32_t sessionIndex, const uint32_t dataSize, char* pData) {
		auto pClient = GetClientInfo(sessionIndex); 
		if (pClient == nullptr) return false; 
		return pClient->SendData(dataSize, pData); 
	}

	virtual void OnClientConnect(const uint32_t clientIndex) {}
	virtual void OnClientDisconnect(const uint32_t clientIndex) {}
	virtual void OnClientReceive(const uint32_t clientIndex,
		const uint32_t size, char* pData) {}

private:
	void CreateClient(const int maxClientCount) {
		for (int i = 0; i < maxClientCount; ++i) {
			_arrClientInfos.emplace_back();
			_arrClientInfos[i].Init(i); 
		}
	}

	bool CreateWorkerThreads() {
		uint32_t thread_id = 0; 
		for (uint32_t i = 0; i < MAX_WORKER_THREAD; ++i) {
			_arrIOWorkerThreads.emplace_back([this]() {
				WorkerThreadProc();
				});
		}
		fprintf_s(stdout, "IOCP %2d Worker Threads Creation Success\n", MAX_WORKER_THREAD);
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
			if (clientInfo.IsConnected() == false) {
				return &clientInfo;
			}
		}
		return nullptr; 
	}

	ClientInfo* GetClientInfo(const uint32_t index) {
		if (index >= _arrClientInfos.size()) return nullptr; 
		return &_arrClientInfos[index]; 
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
				_isWorkerThreadRunning = false;
				continue; 
			}
			if (lpOverlapped == nullptr) {
				continue; 
			}
			if (fCallSuccess == FALSE || IOByteSize == 0) {
				CloseSocketClient(pClientInfo);
				continue; 
			}

			OverlappedEx* pOverlappedEx = (OverlappedEx*)lpOverlapped; 

			if (pOverlappedEx->_ioOperation == IOOperation::RECV) {
				OnClientReceive(
					pClientInfo->GetIndex(),
					IOByteSize,
					pClientInfo->GetRecvBuffer());
				pClientInfo->BindRecv(); 
			}
			else if (pOverlappedEx->_ioOperation == IOOperation::SEND) {
				delete[] pOverlappedEx->_wsaBuffer.buf;
				delete pOverlappedEx; 
				pClientInfo->SendCompleted(IOByteSize);
			}
			else {
				fprintf_s(stderr, "[ERROR] : CLIENT %d Unknown IO Operation.\n", pClientInfo->GetIndex());
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

			SOCKET newClientSocket = accept(_socketListen, (SOCKADDR*)&clientAddr, &clientAddrLen);
			if (newClientSocket  == INVALID_SOCKET) continue;
			
			if (pClientInfo->OnConnect(_iocpHandle, newClientSocket) == false) {
				pClientInfo->Close(); 
				continue; 
			}

			OnClientConnect(pClientInfo->GetIndex()); 

			char clientBufferIP[32] = {}; 
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientBufferIP, sizeof(clientBufferIP));
			fprintf_s(stdout, "[INFO] Client Connected - IP: %s, PORT: %d, SOCKET: %lld\n",
				clientBufferIP,
				ntohs(clientAddr.sin_port),
				newClientSocket);
			++_countClient; 
		}
	}

	void CloseSocketClient(ClientInfo* pClientInfo, bool isEnforced = false) {
		auto clientIndex = pClientInfo->GetIndex(); 
		pClientInfo->Close(isEnforced);
		OnClientDisconnect(clientIndex);
	}

}; 
