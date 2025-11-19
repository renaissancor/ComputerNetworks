#include "stdafx.h"
#include "Network.h" 
#include "Game.h" 

// Networks.cpp 

Network::Manager::Manager() = default;

Network::Manager::~Manager() = default; 

bool Network::Manager::Initialize() noexcept {

	int _WSAStartupResult = 0;
	int _ListenSocketCreateResult = 0;
	int _setsockoptNagleResult = 0;
	int _setsockoptLingerResult = 0;
	int _ioctlsocketResult = 0;
	int _ListenSocketBindResult = 0;
	int _ListenSocketListenResult = 0;

	auto Helper = [&]() noexcept -> bool {
		_WSAGetLastErrorResult = WSAGetLastError();
		::closesocket(_hListenSocket);
		::WSACleanup();
		return false; 
		};

	_WSAStartupResult = ::WSAStartup(MAKEWORD(2, 2), &_wsa); 
	if (_WSAStartupResult != 0) return false;

	_hListenSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	if (_hListenSocket == INVALID_SOCKET) {
		_WSAGetLastErrorResult = WSAGetLastError(); 
		::WSACleanup(); 
		return false; 
	}
		
	int flag = 1;
	_setsockoptNagleResult = ::setsockopt(_hListenSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
	if (_setsockoptNagleResult == SOCKET_ERROR) 
		return Helper();

	u_long nonBlockingMode = 1; 
	_ioctlsocketResult = ::ioctlsocket(_hListenSocket, FIONBIO, &nonBlockingMode);
	if (_ioctlsocketResult == SOCKET_ERROR)
		return Helper();

	linger so_linger = { 0, };
	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	_setsockoptLingerResult = ::setsockopt(_hListenSocket, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger));
	if (_setsockoptLingerResult == SOCKET_ERROR) 
		return Helper();

	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = htons(SERVER_PORT);
	_serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); 

	_ListenSocketBindResult = ::bind(_hListenSocket, 
		(SOCKADDR*)&_serverAddr, sizeof(_serverAddr)); 

	if (_ListenSocketBindResult == SOCKET_ERROR)
		return Helper();

	_ListenSocketListenResult = ::listen(_hListenSocket, SOMAXCONN); 
	if (_ListenSocketListenResult == SOCKET_ERROR) 
		return Helper();

	FD_ZERO(&_masterReadSet);
	FD_SET(_hListenSocket, &_masterReadSet); 

	return true;
}

void Network::Manager::UpdateMaxSocket() noexcept {
	_hMaxSocket = _hListenSocket; 
	for (const auto& sessionPair : _sessions) {
		if (sessionPair.second.hSocket > _hMaxSocket) 
			_hMaxSocket = sessionPair.second.hSocket; 
	}
}

int Network::Manager::AcceptNewClient() noexcept {
	for (int newClient = 0;;++newClient) {
		SOCKADDR_IN clientAddr = { 0 };
		int clientAddrSize = sizeof(clientAddr);
		SOCKET hClient = ::accept(_hListenSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);

		if (hClient == INVALID_SOCKET) {
			int wsaError = WSAGetLastError();
			if (wsaError != WSAEWOULDBLOCK) _WSAGetLastErrorResult = wsaError;
			return newClient;
		}

		Session newSession = { hClient, *(sockaddr*)&clientAddr };
		uint32_t sessionID = InterlockedIncrement(&_sessionIDCount);
		_sessions.emplace(sessionID, newSession);
		FD_SET(hClient, &_masterReadSet);
		if (hClient > _hMaxSocket) _hMaxSocket = hClient;

		u_long nonBlockingMode = 1;
		::ioctlsocket(hClient, FIONBIO, &nonBlockingMode);

		Game::Manager::GetInstance().EnqueueChange(
			Game::ChangeType::LOGIN, sessionID, 0, 0);
		Game::Manager::GetInstance().EnqueueChange(
			Game::ChangeType::CREATE, sessionID, 0, 0);
	}; 
}

std::unordered_map<unsigned int, Network::Session>::iterator
Network::Manager::HandleDisconnection
(std::unordered_map<unsigned int, Session>::iterator sessionIt) noexcept {
	// Close socket and remove from read set 
	::closesocket(sessionIt->second.hSocket);
	FD_CLR(sessionIt->second.hSocket, &_masterReadSet);
	// Remove session from the map and return the next iterator
	return _sessions.erase(sessionIt); 
}

void Network::Manager::RecvStream() noexcept {
	UpdateMaxSocket(); 
	timeval timeout = { 0, 0 }; 
	fd_set readSet = _masterReadSet; 

	int selectCount = ::select((int)_hMaxSocket + 1, &readSet, nullptr, nullptr, &timeout);
	if (selectCount == SOCKET_ERROR || selectCount == 0) {
		_WSAGetLastErrorResult = WSAGetLastError();
		return; 
	}

	if (FD_ISSET(_hListenSocket, &readSet)) {
		AcceptNewClient();
		selectCount--; 
	}

	if (selectCount <= 0) return; 

	for (auto it = _sessions.begin(); it != _sessions.end(); ) {
		Session& session = it->second;
		if (FD_ISSET(session.hSocket, &readSet)) {
			int bytesReceivable = RECV_BUFFER_SIZE - session.recvBufferOffset;
			int bytesReceived = ::recv(session.hSocket, 
				session.recvBuffer + session.recvBufferOffset, bytesReceivable, 0);

			if (bytesReceived > 0) {
				session.recvBufferOffset += bytesReceived; 
				// 16 bytes per Protocol::Packet 
				
				while (session.recvBufferOffset >= sizeof(Protocol::Packet)) {
					Protocol::Packet* packet = 
						reinterpret_cast<Protocol::Packet*>(session.recvBuffer);
					Game::Manager::GetInstance().EnqueueChange(
						static_cast<int>(packet->type),
						packet->id,
						static_cast<int>(packet->x),
						static_cast<int>(packet->y)
					);
					// Shift remaining data in buffer
					int remainingBytes = session.recvBufferOffset - sizeof(Protocol::Packet);
					if (remainingBytes > 0) {
						memmove(session.recvBuffer, 
							session.recvBuffer + sizeof(Protocol::Packet), 
							remainingBytes);
					}
					session.recvBufferOffset = remainingBytes;
				}

			}
			else {
				int wsaError = WSAGetLastError();
				if (bytesReceived == SOCKET_ERROR && wsaError == WSAEWOULDBLOCK) 
				{}
				else {
					_WSAGetLastErrorResult = wsaError; 
					it = HandleDisconnection(it);
					--selectCount;
					continue; 
				}
			} // if bytesReceived > 0 else end 
			--selectCount;
		} // if FD_ISSET end 
		++it; 
		if (--selectCount <= 0) break; 
	}
}

void Network::Manager::SendStream() noexcept {
	while (!_sendStreams.empty()) {
		const SendChange& change = _sendStreams.front();
		Protocol::Packet packet = {
			static_cast<uint32_t>(change.type),
			change.id,
			static_cast<uint32_t>(change.x),
			static_cast<uint32_t>(change.y)
		};
		auto sessionIt = _sessions.find(change.id);
		if (sessionIt != _sessions.end()) {
			Session& session = sessionIt->second;
			int bytesSent = ::send(session.hSocket, 
				reinterpret_cast<const char*>(&packet), sizeof(Protocol::Packet), 0);
			if (bytesSent == SOCKET_ERROR) {
				int wsaError = WSAGetLastError();
				_WSAGetLastErrorResult = wsaError; 
				HandleDisconnection(sessionIt);
			}
		}
		_sendStreams.pop();
	}
}

void Network::Unicast(const int type, const unsigned int id,
	const int x, const int y) noexcept {
	Manager::GetInstance().EnqueueUnicast(type, id, x, y); 
}

void Network::Broadcast(const int type, const unsigned int id,
	const int x, const int y) noexcept {
	Manager::GetInstance().EnqueueBroadcast(type, id, x, y); 
}


void Network::Manager::Shutdown() noexcept {

	if (_hListenSocket != INVALID_SOCKET) {
		::closesocket(_hListenSocket);
		_hListenSocket = INVALID_SOCKET;
	}
	::WSACleanup();
}

