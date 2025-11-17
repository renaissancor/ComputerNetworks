#include "stdafx.h"
#include "gameplay.h"
#include "networks.h" 
#include "keyboard.h"
#include "console.h"

Gameplay::Manager Gameplay::Manager::instance;

Gameplay::Manager::Manager()
{}

Gameplay::Manager::~Manager()
{}

void Gameplay::Manager::Initialize() noexcept {
}

void Gameplay::Manager::MoveLocalPlayerBy(const int dx, const int dy) noexcept {
	// if (dx == 0 && dy == 0) return; 
	Player* localPlayer = nullptr;
	for (const unique_ptr<Player>& player : _players) {
		if (player->id == _localPlayerID) {
			localPlayer = player.get();
			break;
		}
	}
	if (!localPlayer) return;
	int newX = localPlayer->x + dx;
	int newY = localPlayer->y + dy; 
	bool moved = false; 
	if (newX >= 0 && newX < GAME_WIDTH) {
		localPlayer->x = newX;
		moved = true;
	}
	if (newY >= 0 && newY < GAME_HEIGHT) {
		localPlayer->y = newY;
		moved = true;
	}
	if (moved) Networks::Manager::GetInstance().Send(newX, newY);
}

void Gameplay::Manager::Update() noexcept {
	while (!_changes.empty()) {
		Change& change = _changes.front();
		switch (change.type) {
			case Change::LOGIN:
			_localPlayerID = change.id;
			break;
			case Change::CREATE:
				_players.push_back(std::make_unique<Player>(change.id, change.x, change.y));
				break;
			case Change::REMOVE:
				_players.remove_if([&](const unique_ptr<Player>& player) {
					return player->id == change.id;
				});
				break;
			case Change::MOVE:
				for (unique_ptr<Player>& player : _players) {
					if (player->id == change.id) {
						player->x = change.x;
						player->y = change.y;
						break;
					}
				}
				break;
		}
		_changes.pop();
	}
}

void Gameplay::Manager::Render() const noexcept {
	for (const unique_ptr<Player>& player : _players) {		
		Console::Manager::GetInstance().Draw(player->y, player->x, 
			(player->id == _localPlayerID) ? '@' : 'O', 
			(player->id == _localPlayerID) ? 0x0A : 0x0E);
	}
}