#include "stdafx.h"
#include "Game.h"
#include "Network.h" 
#include "Console.h"

// Game.cpp 

Game::Manager::Manager() = default; 
Game::Manager::~Manager() = default; 

void Game::Manager::Update() noexcept {
	while (!_recvStreams.empty()) {
		RecvChange& stream = _recvStreams.front();
		if (stream.type == 0) {
			// LOGIN 
			stream.x = rand() % GAME_WIDTH; 
			stream.y = rand() % GAME_HEIGHT;
			_players.emplace(stream.id, 
				std::make_unique<Player>(stream.id, stream.x, stream.y)); 
			Network::Unicast(LOGIN, stream.id, stream.id, stream.x, stream.y);
			for (const auto& pair : _players) {
				const Player& existingPlayer = *(pair.second);
				Network::Unicast(CREATE, stream.id, existingPlayer.id, existingPlayer.x, existingPlayer.y);
			}
			Network::Broadcast(CREATE, stream.id, stream.id, stream.x, stream.y);
		}
		else if (stream.type == 1) {
			// CREATE, ERROR CASE 
			
		}
		else if (stream.type == 2) {
			// REMOVE 
			_players.erase(stream.id); 
			Network::Broadcast(2, stream.id, stream.id, stream.x, stream.y); 
		}
		else if (stream.type == 3) {
			// MOVE 
			auto playerIt = _players.find(stream.id); 
			if (playerIt != _players.end()) {
				playerIt->second->x = stream.x;
				playerIt->second->y = stream.y;
				if (playerIt->second->x < 0) 
					playerIt->second->x = 0; 
				else if (playerIt->second->x >= GAME_WIDTH) 
					playerIt->second->x = GAME_WIDTH - 1;
				if (playerIt->second->y < 0)
					playerIt->second->y = 0; 
				else if (playerIt->second->y >= GAME_HEIGHT) 
					playerIt->second->y = GAME_HEIGHT - 1;
				if (playerIt->second->x != stream.x || playerIt->second->y != stream.y) {
					stream.x = playerIt->second->x;
					stream.y = playerIt->second->y;
				}
				Network::Broadcast(3, stream.id, stream.id, stream.x, stream.y); 
			}
		}
		_recvStreams.pop();
	}
}

void Game::Manager::Render() noexcept {
	for (const auto& pair : _players) {
		const Player& player = *(pair.second);
		Console::Manager::GetInstance().Draw(player.y, player.x, '@', 0x0A);
	}
}