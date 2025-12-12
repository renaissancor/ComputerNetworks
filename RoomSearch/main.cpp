#include "stdafx.h"

#define FD_SETSIZE 128 // Override default 

unsigned char header[10] =
{ 0xff, 0xee, 0xdd, 0xaa, 0x00, 0x99, 0x77, 0x55, 0x33, 0x11 };

SOCKET maxSock = 0; 

struct Session {
    bool recv_answer = false;
    unsigned int port;
    SOCKET sock;
    sockaddr_in sendAddr;
    sockaddr_in recvAddr;
    char sendBuf[10] = { 0, };
    char recvBuf[1024] = { 0, };

    void Init() {
        memcpy(sendBuf, header, 10);
        sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        BOOL opt = TRUE;
        ::setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(opt));

        recvAddr.sin_family = AF_INET;
        recvAddr.sin_port = htons(port);
        recvAddr.sin_addr.s_addr = INADDR_ANY;
        ::bind(sock, (sockaddr*)&recvAddr, sizeof(recvAddr));

        sendAddr.sin_family = AF_INET;
        sendAddr.sin_port = htons(port);
        sendAddr.sin_addr.s_addr = INADDR_BROADCAST;

		maxSock = (sock > maxSock) ? sock : maxSock; 
    }
};

int main() {
    WSADATA wsa;
    int iResult = ::WSAStartup(MAKEWORD(2, 2), &wsa); 
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
	}

    constexpr int PORT_MIN = 10000;
    constexpr int PORT_MAX = 10100;
    constexpr int PORT_CNT = PORT_MAX - PORT_MIN + 1; // 101 

	fd_set readSet = {}; 
    FD_ZERO(&readSet); 
    std::vector<Session> sessions(PORT_CNT);
    for (int i = 0; i < PORT_CNT; ++i) sessions[i].port = PORT_MIN + i;
    for (Session& s : sessions) s.Init(); 
	for (Session& s : sessions) FD_SET(s.sock, &readSet); 
    for (Session& s : sessions) sendto(s.sock, s.sendBuf, sizeof(s.sendBuf), 0,
        (sockaddr*)&s.sendAddr, sizeof(s.sendAddr));

    timeval timeout = { 2, 0 };
    int n = ::select(maxSock, &readSet, nullptr, nullptr, &timeout);

    if (n <= 0) {
		for (Session& s : sessions) closesocket(s.sock);
        WSACleanup();
		return 0;
    }

    for (Session& session : sessions) {
		SOCKET sock = session.sock;
		if (FD_ISSET(sock, &readSet) == 0) continue;

		sockaddr_in from = {}; 
		int fromLen = sizeof(from); 
        int recvResult = ::recvfrom(sock, session.recvBuf, 1024, 0,
			(sockaddr*)&from, &fromLen);
		if (recvResult <= 0) continue;
        if (memcmp(session.recvBuf, header, 10) == 0) continue; // loopback 
		wchar_t* roomName = (wchar_t*)(session.recvBuf); 
		wprintf_s(L"Found Room: %s at Port: %u\n", roomName, session.port); 
    }

    for (Session& s : sessions) closesocket(s.sock);
    WSACleanup();
    return 0;
}