#pragma once

// Network.h 

using std::string; 
using std::vector; 
using std::queue; 

namespace Network {
	
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
		vector<Log> logs; // Replace to Memory Mapped File later 
		queue<Log> server_data_dummy; // Logs waiting to be written to main log storage 

	private:
		Manager();
		~Manager();
	public:
		inline static Manager& GetInstance() noexcept { return instance; } 
		inline const vector<Log>& GetLogs() const noexcept { return logs; }

		void SendMsg(const string& msg) noexcept; // Send Message to Server, now just store in pending logs 
		void RecvLog(const Log& log) noexcept; // Receive Log from Server 

		void Update() noexcept; 
		void Render() noexcept; 

	};
} // namespace Records