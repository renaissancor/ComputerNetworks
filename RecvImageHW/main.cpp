#include "stdafx.h"

constexpr const unsigned int BUFFER_SIZE = 4096; 
constexpr const wchar_t* FILE_NAME = L"JHPark_photo_recv.jpg";

using namespace std; 

# pragma pack(push, 1)
struct PacketHeader {
	uint32_t packetCode;
	wchar_t username[32];
	wchar_t filename[128];
	uint32_t filesize;
};
# pragma pack(pop) 

WSADATA wsa = { 0 };
SOCKET hListen = INVALID_SOCKET;
SOCKET hClient = INVALID_SOCKET; 
SOCKADDR_IN serverAddr = { 0 }; 
SOCKADDR_IN clientAddr = { 0 }; 

static bool Initiate() {
	if(::WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
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
	serverAddr.sin_port = htons(10010); 
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

	fwprintf_s(stdout, L"Waiting for Client Connection...\n");
	int clientAddrSize = sizeof(clientAddr);
	hClient = ::accept(hListen, (SOCKADDR*)&clientAddr, &clientAddrSize);
	if (hClient == INVALID_SOCKET) {
		fwprintf_s(stderr, L"Client Accept Failed! Error Code : %d\n", WSAGetLastError());
		::closesocket(hListen);
		::WSACleanup();
		return false;
	}

	return true; 
}

static int RecvAll(char* buffer, int len) {
	int totalRecv = 0;
	int remaining = len;
	int bytesRecv = 0; 

	while (totalRecv < len) {
		bytesRecv = ::recv(hClient, buffer + totalRecv, remaining, 0);
		if (bytesRecv == SOCKET_ERROR || bytesRecv == 0) {
			fwprintf_s(stderr, L"Receive Error! Code : %d\n", WSAGetLastError());
			return SOCKET_ERROR; 
		}
		totalRecv += bytesRecv; 
		remaining -= bytesRecv; 
	}

	return totalRecv; 
}

static void Download() {
	unsigned char headerBuffer[sizeof(PacketHeader)] = { 0, };
	int headerRecv = RecvAll((char*)headerBuffer, sizeof(PacketHeader));
	if (headerRecv <= 0) {
		fwprintf_s(stderr, L"Packet Header Receive Failed or Connection Closed! Code: %d\n", WSAGetLastError());
		return;
	}

	PacketHeader* pHeader = (PacketHeader*)headerBuffer;
	fwprintf_s(stdout, L"Received Packet Header Info : \n");
	fwprintf_s(stdout, L" Packet Code : 0x%08X\n", pHeader->packetCode);
	fwprintf_s(stdout, L" Username    : %ls\n", pHeader->username);
	wmemcpy_s(pHeader->filename, sizeof(pHeader->filename) / sizeof(wchar_t),
		FILE_NAME, wcslen(pHeader->filename));
	fwprintf_s(stdout, L" Filename    : %ls\n", pHeader->filename);


	
	FILE* pf = nullptr;
	_wfopen_s(&pf, pHeader->filename, L"wb");
	if (pf == nullptr) {
		fwprintf_s(stderr, L"File Open Failed! : %ls\n", pHeader->filename);
		return;
	}

	unsigned char* buffer = new unsigned char[4096];

	size_t totalReceived = 0;
	while (totalReceived < pHeader->filesize) {
		size_t remainingSize = pHeader->filesize - totalReceived;
		int bytesToRecv = (remainingSize > BUFFER_SIZE) ? BUFFER_SIZE : (int)remainingSize;
		int recvBytes = RecvAll((char*)buffer, bytesToRecv);

		if (recvBytes <= 0) {
			fwprintf_s(stderr, L"File Data Receive Error! Error Code : %d\n", WSAGetLastError());
			break; // ·çÇÁ Å»Ãâ
		}

		fwrite(buffer, 1, recvBytes, pf);
		totalReceived += recvBytes;
		fwprintf_s(stdout, L"\rReceived %zu / %u bytes", totalReceived, pHeader->filesize);
	}

	fwprintf_s(stdout, L"\nFile Receive Completed!\n");
	if (pf) fclose(pf);

	uint32_t responseCode = htonl(0xdddddddd); 
	::send(hClient, (const char*)&responseCode, sizeof(responseCode), 0); 

	delete[] buffer;
}

int main() {
	
	if (Initiate() == false) return 0; // Initiation Failed 
	Download(); 

	::closesocket(hClient);
	::closesocket(hListen);
	::WSACleanup();
	
	
	return 0; 
}