#include "stdafx.h"

// Select based MultiPlexing Server Example 

using namespace std; 

constexpr const wchar_t* SERVER_IP = L"127.0.0.1"; // Loopback Address 
constexpr const unsigned short SERVER_PORT = 9000;

struct ClientInfo {
	SOCKET hClient;
	SOCKADDR_IN clientAddr;
};

WSADATA wsa = { 0 };
SOCKET hListen = INVALID_SOCKET;
SOCKADDR_IN serverAddr = { 0 };
char* buffer = nullptr; 

vector<ClientInfo> clients;


static bool Initiate() {
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		fwprintf_s(stderr, L"WSAStartup Failed! Error Code : %d\n", WSAGetLastError());
		return false;
	}
	hListen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (hListen == INVALID_SOCKET) {
		fwprintf_s(stderr, L"Listen Socket Create Failed! Error Code : %d\n", WSAGetLastError());
		::WSACleanup();
		return false;
	}

	int flag = 1; // Disable Nagle's Algorithm 
	::setsockopt(hListen, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
	linger so_linger = { 0, }; // Set Linger Option Close by RST, NO 4 Way Handshake 
	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	::setsockopt(hListen, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	int bindResult = ::bind(hListen, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (bindResult == SOCKET_ERROR) {
		fwprintf_s(stderr, L"Listen Socket Bind Failed! Error Code : %d\n", WSAGetLastError());
		::closesocket(hListen);
		::WSACleanup();
		return false;
	}

	int listenResult = ::listen(hListen, SOMAXCONN);
	if (listenResult == SOCKET_ERROR) {
		fwprintf_s(stderr, L"Socket Listen Failed! Error Code : %d\n", WSAGetLastError());
		::closesocket(hListen);
		::WSACleanup();
		return false;
	}
	buffer = new char[65536];

	return true;
}

static void Terminate() {
	for (auto& client : clients) 
		::closesocket(client.hClient);
	::closesocket(hListen);
	::WSACleanup();
	delete[] buffer;
}



int main(int argc, char* argv[]) {
	if (!Initiate()) return -1;

	fd_set masterSet = { 0, };
	FD_ZERO(&masterSet); 
	FD_SET(hListen, &masterSet); 

	SOCKET maxSocket = hListen; 
	fwprintf_s(stdout, L"Select Multiplexing Server Started. Waiting for Connections...\n");
	while (true) {
		fd_set readSet = masterSet; 
		int select_result = ::select((int)(maxSocket + 1), &readSet, nullptr, nullptr, nullptr);
		if (select_result == SOCKET_ERROR) {
			fwprintf_s(stderr, L"Select Error! Error Code : %d\n", WSAGetLastError());
			break;
		}

		if (FD_ISSET(hListen, &readSet)) {
			SOCKADDR_IN clientAddr = { 0 }; 
			int clientAddrSize = sizeof(clientAddr); 
			SOCKET hClient = ::accept(hListen, (SOCKADDR*)&clientAddr, &clientAddrSize);
			
			if(hClient == INVALID_SOCKET) {
				fwprintf_s(stderr, L"Client Accept Failed! Error Code : %d\n", WSAGetLastError());
				continue; 
			}

			FD_SET(hClient, &masterSet);
			clients.push_back({ hClient, clientAddr }); 
			if (hClient > maxSocket) {
				maxSocket = hClient; 
			}

			char clientIP[INET_ADDRSTRLEN] = { 0, };
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
			fwprintf_s(stdout, L"New Client Connected: %hs:%d\n", 
				clientIP, ntohs(clientAddr.sin_port));
			--select_result;
		}

		memset(buffer, 0, 65536); 
		for(auto it = clients.begin(); it != clients.end() && select_result > 0; ) {
			SOCKET hClient = it->hClient; 
			if (FD_ISSET(hClient, &readSet)) {
				int recvBytes = ::recv(hClient, buffer, 65536, 0);
				if (recvBytes > 0) {
					fwprintf_s(stdout, L"Received %d bytes from %lld. Broadcasting...\n", recvBytes, hClient);
					for (const auto& client : clients) {
						::send(client.hClient, buffer, recvBytes, 0);
					}
				}
				else {
					char clientIP[INET_ADDRSTRLEN] = { 0, };
					inet_ntop(AF_INET, &it->clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
					if (recvBytes == 0) {
						fwprintf_s(stdout, L"Client Disconnected (Graceful): %hs:%d\n", clientIP, ntohs(it->clientAddr.sin_port));
					}
					else {
						fwprintf_s(stderr, L"Client Disconnected (Error %d): %hs:%d\n", WSAGetLastError(), clientIP, ntohs(it->clientAddr.sin_port));
					}
					FD_CLR(hClient, &masterSet); 
					::closesocket(hClient);
					it = clients.erase(it);
				}
				--select_result;
			}
			else {
				++it; 
			}
		}
	}

	return 0; 
}