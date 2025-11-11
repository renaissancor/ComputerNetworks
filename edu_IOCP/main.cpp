#include "stdafx.h"
#include "Types.h"
#include "IOCP.h" 

constexpr const unsigned int SERVER_PORT = 9000; 
constexpr const unsigned int MAX_CLIENT_COUNT = 100;

int main()
{
	IOCompletionPort iocp;
	iocp.InitSocket();
	iocp.BindAndListen(SERVER_PORT);
	iocp.StartIOCPServer(MAX_CLIENT_COUNT); 

	fprintf_s(stdout, "Press Enter Key to Exit...\n"); 
	getchar(); 
	iocp.DestroyThreads(); 


	return 0;
}