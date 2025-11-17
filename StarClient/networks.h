#pragma once

namespace Networks {
	constexpr const wchar_t* SERVER_IP = L"192.168.20.33"; // For Convenience  
	constexpr const u_short SERVER_PORT = 3000;

	constexpr const int RECV_BUFFER_SIZE = 4096; 

	struct NetworkResults {
		int _isConnectedToServer = 0; 

		int _WSALastErrorResult = 0;
		int _InetPtonWResult = 0;
		int _WSAStartupResult = 0;
		int _SocketCreateResult = 0;
		int _setsockoptResult = 0;
		int _ioctlsocketResult = 0;

		int _connectResult = 0;
		int _sendResult = 0;
		int _recvResult = 0;
	};

	class Manager {
	private: 
		Manager();
		~Manager();
		Manager(Manager const&) = delete;
		Manager const& operator=(Manager const&) = delete; 
	private:
		static Manager instance; 
		WSADATA _wsa = { 0 };
		SOCKET _hSocket = INVALID_SOCKET;
		SOCKADDR_IN _serverAddr = { 0 }; 
		wchar_t _ipAddr[INET_ADDRSTRLEN] = { 0 }; 
		NetworkResults _results;

		char _recvBuffer[RECV_BUFFER_SIZE] = { 0 }; 
		int _recvBufferOffset = 0; 

	public:
		inline static Manager& GetInstance() noexcept { return instance; }
	public:
		bool Initialize() noexcept;
		bool Connect() noexcept; 
		void Shutdown() noexcept;

		void Send(const int x, const int y) noexcept; 
		void Recv() noexcept; 
	}; 
}