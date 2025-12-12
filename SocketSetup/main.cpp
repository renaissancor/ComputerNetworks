#include "stdafx.h"

unsigned char header[10] =
{ 0xff, 0xee, 0xdd, 0xaa, 0x00, 0x99, 0x77, 0x55, 0x33, 0x11 };

int main() {
	WSADATA wsa; 
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsa); 
	if (iResult) return -1; 

	std::vector<int> a(10, 99); 
	std::vector<int>::iterator it = a.begin(); 
	std::cout << sizeof(it) << std::endl; 

	std::unordered_map<int, std::string> hashmap; 
	std::unordered_map<int, std::string>::iterator hit = hashmap.begin(); 
	std::unordered_map<int, std::string>::const_iterator chit = hashmap.cbegin(); 
	std::cout << sizeof(hit) << std::endl; 
	std::cout << sizeof(chit) << std::endl;

	SOCKET maxSocket = INVALID_SOCKET; 

	std::vector<SOCKET> sockets(101, INVALID_SOCKET); 
	for (int i = 0; i < 101; ++i) {
		SOCKET& sock = sockets[i]; 
		sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); 

		int opt = 1; 
		::setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)opt, opt); 

		sockaddr_in thisAddr = {}; 
		thisAddr.sin_family = AF_INET; 
		thisAddr.sin_port = htons((u_short)i + 10000); 
		thisAddr.sin_addr.s_addr = INADDR_ANY; // My Whatever IP 
		
		sockaddr_in otherAddr = {}; 
		otherAddr.sin_family = AF_INET; 
		// otherAddr.sin_port // no need to set up this 
		otherAddr.sin_addr.s_addr = INADDR_BROADCAST; 

		int otherAddrLen = sizeof(otherAddr); 
		::sendto(sock, (const char*)header, 10, 0, (sockaddr*) & otherAddr, sizeof(otherAddr)); 

		maxSocket = sock < maxSocket ? maxSocket : sock; 
	}

	fd_set readSet = {}; 
	FD_ZERO(&readSet); 
	timeval timeout = { 1, 0 }; // block for 1 seconds 
	int n = ::select(maxSocket, &readSet, nullptr, nullptr, &timeout); 
		


	return 0; 
}