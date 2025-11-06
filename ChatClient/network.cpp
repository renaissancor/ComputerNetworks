#include "stdafx.h"
#include "Network.h"
#include "console.h"

// Network.cpp 

Network::Manager Network::Manager::instance; 

Network::Manager::Manager()
	: logs(), server_data_dummy()
{
}

Network::Manager::~Manager()
{
}

void Network::Manager::SendMsg(const string& msg) noexcept { 
	// Send Message to Server by Network code 
	// For now, just store the log in pending logs 
	Log new_log;
	new_log.message = msg;
	new_log.timestamp = static_cast<long long>(GetTickCount64());
	new_log.id = logs.size() > 0 ? logs.back().id + 1 : 1;
	server_data_dummy.push(new_log);
} 

void Network::Manager::RecvLog(const Log& log) noexcept { 
	// Receive Log from Server
	// For now, just store the log in pending logs 
	if (server_data_dummy.empty()) return; 
	Log new_log = server_data_dummy.front(); 
	server_data_dummy.pop(); 

	logs.push_back(new_log); 
}

void Network::Manager::Update() noexcept { 
	while (!server_data_dummy.empty()) {
		Log new_log = server_data_dummy.front();
		server_data_dummy.pop();
		logs.push_back(new_log);
	}
}

void Network::Manager::Render() noexcept { 
	Console::Manager& render_manager = Console::Manager::GetInstance(); 
	size_t log_count = logs.size(); 
	size_t start_index = log_count >= 20 ? log_count - 20 : 0; 
	for (size_t i = 0; i < 20; i++) {
		size_t log_index = start_index + i; 
		if (log_index < log_count) {
			const Log& log = logs[log_index]; 
			string line = "[" + std::to_string(log.id) + "] " + log.message; 
			render_manager.draw_line(static_cast<short>(i + 3), line.c_str()); 
		}
		else {
			render_manager.draw_line(static_cast<short>(i + 3), ""); 
		}
	}
	
}
