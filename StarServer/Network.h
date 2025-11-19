#pragma once

// Network.h

namespace Network {
	constexpr const u_short SERVER_PORT = 3000; 
	constexpr const int RECV_BUFFER_SIZE = 1024;
	// constexpr const int SEND_BUFFER_SIZE = 4096; 

	struct SendChange {
		int type = 0;
		unsigned int id = 0;
		int x = 0;
		int y = 0;
	};

	struct Session {
		SOCKET hSocket = INVALID_SOCKET; 
		sockaddr addrClient = { 0 }; 
		int recvBufferOffset = 0; 
		char recvBuffer[RECV_BUFFER_SIZE] = { 0 }; 
	};

	

	class Manager {
	private:
		Manager();
		~Manager();
		Manager(Manager const&) = delete;
		Manager const& operator=(Manager const&) = delete;
	private:
		WSADATA _wsa = { 0 };
		SOCKET _hListenSocket = INVALID_SOCKET; 
		SOCKET _hMaxSocket = INVALID_SOCKET; 
		SOCKADDR_IN _serverAddr = { 0 }; 

		int _WSAGetLastErrorResult = 0;

		volatile unsigned int _sessionIDCount = 0;

		std::unordered_map<unsigned int, Session> _sessions;

		std::queue<SendChange> _sendStreams; 

		fd_set _masterReadSet = { 0, }; 

	private:
		void UpdateMaxSocket() noexcept; 
		int AcceptNewClient() noexcept; 
		std::unordered_map<unsigned int, Session>::iterator HandleDisconnection
			(std::unordered_map<unsigned int, Session>::iterator sessionIt) noexcept; 

	public:
		inline static Manager& GetInstance() noexcept {
			static Manager instance;
			return instance;
		}

		inline void EnqueueUnicast (const int type, const unsigned int id,
			const int x, const int y) noexcept {
			_sendStreams.push(SendChange{ type, id, x, y });
		}

		inline void EnqueueBroadcast(const int type, const unsigned int id,
			const int x, const int y) noexcept {
			for (auto& sessionPair : _sessions) {
				if (sessionPair.first == id) continue;
				_sendStreams.push(SendChange{ type, id, x, y });
			}
		}

		bool Initialize() noexcept;
		void RecvStream() noexcept; 
		void SendStream() noexcept; 
		void Shutdown() noexcept; 

	}; // class Manager 

	void Unicast(const int type, const unsigned int id,
		const int x, const int y) noexcept;
	void Broadcast(const int type, const unsigned int id,
		const int x, const int y) noexcept;
}
