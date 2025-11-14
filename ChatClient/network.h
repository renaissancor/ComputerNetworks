#pragma once

// Network.h 

using std::string; 
using std::vector; 
using std::queue; 

namespace Network {
	
	constexpr const wchar_t* SERVER_IP = L"127.0.0.1"; // Loopback Address 
	constexpr const unsigned short SERVER_PORT = 9000;

	constexpr const size_t SEND_BUFFER_SIZE = 65536; 
	constexpr const size_t RECV_BUFFER_SIZE = 65536;

	struct Log { // Recv From Server 
		long long id = 0; // Unique Log ID 
		long long timestamp = 0; 

		string message;

		string ip_address; // Optional 
		unsigned short port = 0; // Optional 
	};

	class Manager {
	private:
		static Manager instance;
		Manager(Manager const&) = delete;
		Manager const& operator=(Manager const&) = delete;

	private:
		alignas(64)
		WSADATA _wsa; 
		SOCKET _hSocket; 
		SOCKADDR_IN _server_addr;

	private:
		Manager();
		~Manager();
	public:
		inline static Manager& GetInstance() noexcept { return instance; } 
		inline const SOCKET GetSocket() const noexcept { return _hSocket; } 

		bool Initiate() noexcept;
		void Shutdown() noexcept;

		void SendMsg(const string& msg) noexcept; // Send Message to Server, now just store in pending logs 
		void SendMsg(const char* format, ...) noexcept; // Send Formatted Message to Server 
		void SendDummyData() noexcept; // For Test 
		void RecvMsg(const Log& log) noexcept; // Receive Log from Server 

		void Update() noexcept; 
		void Render() noexcept; 

	}; // class Manager 
} // namespace Records