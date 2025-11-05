#pragma once

// network.h 

namespace Network {

	constexpr const wchar_t* SERVER_IP = L"127.0.0.1"; // Loopback Address 
	constexpr const unsigned short SERVER_PORT = 9000;

	class Manager {
	private:
		static Manager instance;
		Manager(Manager const&) = delete;
		Manager const& operator=(Manager const&) = delete;
	private:
		WSAData _wsa;
		SOCKET _hSocket;
		SOCKADDR_IN _server_addr;
	private: // Singleton Pattern 
		Manager();
		~Manager();
	public:
		inline static Manager& GetInstance() noexcept { return instance; }
		inline const SOCKET GetSocket() const noexcept { return _hSocket; } 

		bool Initiate() noexcept; 
		void Shutdown() noexcept; 
		bool SendPacket(const char* data, size_t length) noexcept; 
	};
} // namespace Network 