#include "stdafx.h" 

using namespace std; 
constexpr const wchar_t* SERVER_IP = L"127.0.0.1";
// constexpr const wchar_t* SERVER_IP = L"192.168.20.29";
constexpr const wchar_t* USER_NAME = L"StephenJHPark";
constexpr const wchar_t* FILE_NAME = L"JHPark_Photo.jpg";
constexpr const unsigned short SERVER_PORT = 10010; 
constexpr const unsigned int PACKET_CODE = 0x11223344;

# pragma pack(push, 1)
struct PacketHeader {
	uint32_t packetCode; 
	wchar_t username[32]; 
	wchar_t filename[128];
	uint32_t filesize; 
};
# pragma pack(pop) 

// Global Variables 

WSADATA wsa = { 0 };
SOCKET hSocket = INVALID_SOCKET;
SOCKADDR_IN serverAddr = { 0 };

static bool Initiate() {
	if(::WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
		fwprintf_s(stderr, L"WSAStartup Failed! Error Code : %d\n", WSAGetLastError());
		return false; 
	}

	hSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	if (hSocket == INVALID_SOCKET) {
		fwprintf_s(stderr, L"Socket Create Failed! Error Code : %d\n", WSAGetLastError());
		::WSACleanup();
		return false;
	}

	int flag = 1; // Disable Nagle's Algorithm 
	::setsockopt(hSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
	linger so_linger = { 0, }; // Set Linger Option 
	so_linger.l_onoff = 1;	   // Close by RST not 4 Way Handshake 
	so_linger.l_linger = 0;
	::setsockopt(hSocket, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger));

	serverAddr.sin_family = AF_INET; 
	serverAddr.sin_port = htons(SERVER_PORT); 
	::InetPtonW(AF_INET, SERVER_IP, &serverAddr.sin_addr); 

	int connectResult = ::connect(hSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)); 
	if (connectResult == SOCKET_ERROR) {
		fwprintf_s(stderr, L"Socket Connect Failed! Error Code : %d\n", WSAGetLastError());
		::closesocket(hSocket);
		::WSACleanup();
		return false;
	}

	return true; 
}

static int SendAll (const char* buffer, int len) {
	// To Be Implemented 
	int totalSent = 0;
	int remaining = len; 
	int sentBytes = 0; 
	while (totalSent < len) {
		sentBytes = ::send(hSocket, buffer + totalSent, remaining, 0); 
		if (sentBytes == SOCKET_ERROR) {
			fwprintf_s(stderr, L"Send Error! Code : %d\n", WSAGetLastError());
			return SOCKET_ERROR; 
		}
		totalSent += sentBytes; 
		remaining -= sentBytes; 
	}
	return totalSent; 
}

int main()
{
	FILE* pf = nullptr;
	_wfopen_s(&pf, FILE_NAME, L"rb");
	if (pf == nullptr) {
		fwprintf_s(stderr, L"File Open Failed! : %ls\n", FILE_NAME);
		return 0;
	}
	if (Initiate() == false) {
		fclose(pf); 
		return 0;
	}
	size_t fileSize = 0;
	fseek(pf, 0, SEEK_END);
	fileSize = ftell(pf);
	fseek(pf, 0, SEEK_SET);
	

	PacketHeader header = { 0, };
	header.packetCode = PACKET_CODE; // htonl(PACKET_CODE); 
	wmemcpy_s(header.username, sizeof(header.username) / sizeof(wchar_t), 
		USER_NAME, sizeof(header.username) / sizeof(wchar_t));
	wmemcpy_s(header.filename, sizeof(header.filename) / sizeof(wchar_t),
		FILE_NAME, sizeof(header.filename) / sizeof(wchar_t));
	header.filesize = (uint32_t)fileSize;

	unsigned char buffer[1000] = { 0, };
	memcpy_s(buffer, sizeof(buffer), &header, sizeof(header)); 
	
	if(SendAll((const char*)buffer, sizeof(header)) == SOCKET_ERROR) {
		fwprintf_s(stderr, L"Header Send Failed!\n");
		fclose(pf); 
		::closesocket(hSocket);
		::WSACleanup();
		return 0; 
	}

	size_t bytesRead = 0;
	while ((bytesRead = fread_s(buffer, sizeof(buffer), 1, sizeof(buffer), pf)) > 0) {
		if (SendAll((const char*)buffer, (int)bytesRead) == SOCKET_ERROR) {
			fwprintf_s(stderr, L"File Data Send Failed!\n");
			fclose(pf);
			::closesocket(hSocket);
			::WSACleanup();
			return 0;
		}
	}

	int recvBytes = ::recv(hSocket, (char*)buffer, sizeof(uint32_t), 0);

	if (recvBytes == sizeof(uint32_t)) {
		uint32_t responseCode = *(uint32_t*)buffer;
		// responseCode = ntohl(responseCode); 
		fwprintf_s(stdout, L"Server Response : 0x%08X\n", responseCode);
	}
	else if (recvBytes > 0) {
		fwprintf_s(stderr, L"Server Response Incomplete: Received %d bytes\n", recvBytes);
	}
	else if (recvBytes == 0) {
		fwprintf_s(stderr, L"Server closed connection before sending response.\n");
	}
	else { // SOCKET_ERROR
		fwprintf_s(stderr, L"Receive Error! Code: %d\n", WSAGetLastError());
	}
	
	::closesocket(hSocket);
	::WSACleanup(); 
    
	return 0; 
}
