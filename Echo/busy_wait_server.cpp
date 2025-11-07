#include "stdafx.h"
// Server Echo 
using namespace std;

constexpr const wchar_t* SERVER_IP = L"192.168.0.1"; // Loopback Address 
constexpr const unsigned short SERVER_PORT = 9000;

struct ClientInfo {
	SOCKET hSocket;
	SOCKADDR_IN client_addr;
};

int main()
{
	fprintf_s(stdout, "Process Server Start!\n");

	WSADATA wsa = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		fprintf_s(stderr, "WSAStartup Failed! Error Code : %d\n", WSAGetLastError());
		return -1;
	}

	SOCKET hListenSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (hListenSocket == INVALID_SOCKET)
	{
		fprintf_s(stderr, "Socket Create Failed! Error Code : %d\n", WSAGetLastError());
		::WSACleanup();
		return -1;
	}

	int opt = 1;
	setsockopt(hListenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

	SOCKADDR_IN server_addr = { 0 };
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY; // Allow All IP 
	// InetPton(AF_INET, SERVER_IP, &server_addr.sin_addr); // Only SERVER_IP 

	if (::bind(hListenSocket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
	{
		fprintf_s(stderr, "Socket Bind Failed! Error Code : %d\n", WSAGetLastError());
		closesocket(hListenSocket);
		::WSACleanup();
		return -1;
	}

	if (::listen(hListenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		fprintf_s(stderr, "Socket Listen Failed! Error Code : %d\n", WSAGetLastError());
		::closesocket(hListenSocket);
		::WSACleanup();
		return -1;
	}

	SOCKADDR_IN client_addr = { 0 };
	int addr_len = sizeof(client_addr);
	vector<ClientInfo> clients_info;
	char recv_buf[65536] = { 0 };
	char send_buf[65536] = { 0 };

	u_long mode = 1;
	ioctlsocket(hListenSocket, FIONBIO, &mode); // Non-Blocking Mode 

	while (true) {
		SOCKET hNewClientSocket = ::accept(hListenSocket, (SOCKADDR*)&client_addr, &addr_len);
		if (hNewClientSocket != INVALID_SOCKET) {
			// New Client Connected 
			u_long mode = 1; 
			ioctlsocket(hNewClientSocket, FIONBIO, &mode); 
			clients_info.push_back({ hNewClientSocket , client_addr });
			char client_ip[INET_ADDRSTRLEN] = { 0 };
			inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
			fprintf_s(stdout, "Client Connected: %s:%d\n",
				client_ip, ntohs(client_addr.sin_port));
		}

		for (auto client = clients_info.begin();
			client != clients_info.end(); /**/) {
			int recv_status = ::recv(client->hSocket, recv_buf, sizeof(recv_buf), 0);
			if (recv_status > 0) {
				memcpy(send_buf, recv_buf, recv_status);

				for (auto other_client = clients_info.begin();
					other_client != clients_info.end(); ++other_client) {
					// if (other_client->hSocket != client->hSocket) 
					::send(other_client->hSocket, send_buf, recv_status, 0);
						
					
				}
				fprintf_s(stdout, "%.*s\n", recv_status, recv_buf);
				memset(recv_buf, 0, sizeof(recv_buf));
				++client;
			}
			else if (recv_status == 0) {
				fprintf_s(stdout, "Client gracefully requested disconnect\n");
				::shutdown(client->hSocket, SD_BOTH);
				::closesocket(client->hSocket);
				client = clients_info.erase(client);
			}
			else {
				int err = WSAGetLastError();
				if (err == WSAEWOULDBLOCK) {
					++client;
					continue;
				}
				else {
					fprintf_s(stderr, "Client Recv error! Code: %d\n", err);
					::shutdown(client->hSocket, SD_BOTH);
					::closesocket(client->hSocket);
					client = clients_info.erase(client);
				}
			}
		}
		Sleep(1);
	}

	::closesocket(hListenSocket);
	::WSACleanup();

	return 0;
}
