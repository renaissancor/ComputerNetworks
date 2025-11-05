#include "stdafx.h"
#include "network.h"
#include "console.h" 
#include "datalog.h" 

using namespace std; 

volatile LONG running = 2'147'483'647;


static unsigned __stdcall recv_thread_func(void* param)
{
	Network::Manager& network_mgr = Network::Manager::GetInstance();  
	Console::Manager& console_mgr = Console::Manager::GetInstance(); 
	Log::Manager& log_mgr = Log::Manager::GetInstance(); 

	while (running) {
		char buffer[4096];
		int bytes_received = recv(network_mgr.GetSocket(), buffer, sizeof(buffer) - 1, 0);
		if (bytes_received > 0) {
			buffer[bytes_received] = '\0';

			int required_size = MultiByteToWideChar(CP_ACP, 0, buffer, -1, nullptr, 0);
			std::wstring wstr_recv(required_size, L'\0');
			MultiByteToWideChar(CP_ACP, 0, buffer, -1, &wstr_recv[0], required_size);
			
			log_mgr.Record(L"[Server] %s", wstr_recv.c_str());
		}
		else if (bytes_received == 0) {
			// Connection closed 
			break;
		}
		else {
			// Error occurred 
			fprintf_s(stderr, "Receive failed! Error Code : %d\n", WSAGetLastError());
			break;
		}
	}
	return 0;
}


int main(int argc, char *argv[])
{
	Network::Manager& network_mgr = Network::Manager::GetInstance();
	Console::Manager& console_mgr = Console::Manager::GetInstance();

	if (!Console::Manager::GetInstance().Initiate()) {
		fprintf_s(stderr, "Console Manager Initiation Failed!\n");
		return -1;
	}
	if (!Network::Manager::GetInstance().Initiate()) {
		fprintf_s(stderr, "Network Manager Initiation Failed!\n");
		console_mgr.Shutdown(); 
		return -1;
	}

	HANDLE hRecvThread = (HANDLE)_beginthreadex(
		nullptr, 0, &recv_thread_func, nullptr, 0, nullptr); 
	
	if (hRecvThread == nullptr) {
		fprintf_s(stderr, "Receive Thread Creation Failed!\n");
		network_mgr.Shutdown();
		console_mgr.Shutdown();
		return -1;
	}

	while (running) {
		wstring input_line; 
		console_mgr.RefreshInputLine(L"");
		if (input_line == L"/quit") {
			running = 0;
		} 

	}

	::WaitForSingleObject(hRecvThread, INFINITE);
	::CloseHandle(hRecvThread);
	network_mgr.Shutdown();
	console_mgr.Shutdown(); 

    return 0; 
}
