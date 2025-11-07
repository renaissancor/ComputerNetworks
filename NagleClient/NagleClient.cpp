// NagleClient.cpp 
//

#include "stdafx.h"

constexpr const wchar_t* SERVER_IP = L"192.168.20.35"; 
constexpr const unsigned short SERVER_PORT = 9000;

WSADATA wsa = { 0 };
SOCKET hSocket = INVALID_SOCKET;
SOCKADDR_IN server_addr = { 0 }; 
static bool Initiate() {
	// Initialize Network Code Here 
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		fprintf_s(stderr, "WSAStartup Failed! Error Code : %d\n", WSAGetLastError()); 
		return false; 
	}
	hSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	if (hSocket == INVALID_SOCKET) {
		fprintf_s(stderr, "Socket Create Failed! Error Code : %d\n", WSAGetLastError());
		::WSACleanup(); 
		return false; 
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	::InetPtonW(AF_INET, SERVER_IP, &server_addr.sin_addr); 
	int connect_result = ::connect(hSocket, (SOCKADDR*)&server_addr, sizeof(server_addr)); 
	if (connect_result == SOCKET_ERROR) {
		fprintf_s(stderr, "Socket Connect Failed! Error Code : %d\n", WSAGetLastError()); 
		::closesocket(hSocket); 
		::WSACleanup(); 
		return false; 
	}
	return true;
}

int main()
{
	if (!Initiate()) return 0; 

	int flag = 1;
	::setsockopt(hSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)); // Disable Nagle's Algorithm 

	// const char *data = "abcdefghijklmonpqrstuvwxyz";
	const char* data = "abcde";
	char buffer[1460];
	memcpy(buffer, data, sizeof(buffer)); 

	buffer[(sizeof(data))] = '\0'; 
	// 1 to 1 accept test 
	for (int i = 0; i < 1000; ++i) {
		// ::send(hSocket, buffer, 24, 0);
		::send(hSocket, buffer, 1, 0); 
		Sleep(0);	 
	}

	while (true) {
		// If Pressed ESC Key, break
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			break;
		}
		else {
			Sleep(100);
		}
	}

	::closesocket(hSocket);
	::WSACleanup();

	return 0; 
}
