// NagleServer.cpp 
//

#include "stdafx.h"

constexpr const wchar_t* SERVER_IP = L"192.168.20.35";
constexpr const unsigned short SERVER_PORT = 9000;

using std::vector; 

WSADATA wsa = { 0 };

SOCKET hSocket = INVALID_SOCKET;
SOCKET hClient = INVALID_SOCKET; 

SOCKADDR_IN server_addr = { 0 };
SOCKADDR_IN client_addr = { 0 };

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

	// Server bind 
	if (::bind(hSocket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
	{
		fprintf_s(stderr, "Socket Bind Failed! Error Code : %d\n", WSAGetLastError());
		closesocket(hSocket);
		::WSACleanup();
		return false;
	}

	if(::listen(hSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		fprintf_s(stderr, "Socket Listen Failed! Error Code : %d\n", WSAGetLastError());
		::closesocket(hSocket);
		::WSACleanup();
		return false;
	}

	int addr_len = sizeof(client_addr);
	hClient = ::accept(hSocket, (sockaddr*)&client_addr, &addr_len);

	if (hClient == INVALID_SOCKET)
	{
		fprintf_s(stderr, "Socket Accept Failed! Error Code : %d\n", WSAGetLastError());
		::closesocket(hSocket);
		::WSACleanup();
		return false;
	}

	// Non blocking mode
	u_long mode = 1;

	if (::ioctlsocket(hSocket, FIONBIO, &mode) == SOCKET_ERROR) {
		fprintf_s(stderr, "ioctlsocket Failed! Error Code : %d\n", WSAGetLastError());
		::closesocket(hClient);
		::closesocket(hSocket);
		::WSACleanup();
		return false;
	}

	if (::ioctlsocket(hClient, FIONBIO, &mode) == SOCKET_ERROR) {
		fprintf_s(stderr, "ioctlsocket Failed! Error Code : %d\n", WSAGetLastError());
		::closesocket(hClient);
		::closesocket(hSocket);
		::WSACleanup();
		return false;
	}

	return true;
}

struct ClientInfo {
	SOCKET hSocket;
	SOCKADDR_IN client_addr;
};


int main()
{
	if (!Initiate()) return -1; 

	char buffer[14600];
	int recv_len = 0; 
	fprintf_s(stdout, "Start Receiving Data from Client...\n"); 
	while (true) {
		// if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;
		if(hClient == INVALID_SOCKET) {
			hClient = ::accept(hSocket, (sockaddr*)&client_addr, nullptr);
		}
		if (hClient == INVALID_SOCKET) {
			Sleep(10); // Avoid Busy Waiting
			continue;
		}
		recv_len = ::recv(hClient, buffer, sizeof(buffer), 0);
		if (recv_len > 0) {
			// Process Received Data 
			fprintf_s(stdout, "Received %d bytes from Client.\n", recv_len); 
			// fprintf_s(stdout, "Data: %.*s\n", recv_len + 10, buffer); 
		}
		else if (recv_len == 0) {
			// Connection Closed Gracefully 
			fprintf_s(stdout, "Client Disconnected.\n"); 
			::closesocket(hClient); 
			hClient = INVALID_SOCKET; 
			continue; 
			// break; 
		}
		else {
			int error = WSAGetLastError();
			// Non blocking socket did not receive data 
			if (error == WSAEWOULDBLOCK) {
				Sleep(10); // Avoid Busy Waiting 
				continue; 
			}
			else {
				fprintf_s(stderr, "Receive Failed! Network Error Code : %d. Closing hClient.\n", error);
				::closesocket(hClient);
				hClient = INVALID_SOCKET; 
				continue;
			}
		}
		
	}

	::closesocket(hClient);
	::closesocket(hSocket);
	::WSACleanup();

	return 0;
}
