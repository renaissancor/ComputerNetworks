#pragma once

namespace Game {
	constexpr const int GAME_WIDTH = 81;
	constexpr const int GAME_HEIGHT = 24;

	enum ChangeType {
		LOGIN = 0,
		CREATE = 1,
		REMOVE = 2,
		MOVE = 3
	};

	struct RecvChange {
		int type = 0;
		unsigned int id = 0;
		int x = 0;
		int y = 0;
	};

	

	struct Player {
		unsigned int id = 0;
		int x = 0;
		int y = 0;
		Player(unsigned int id_, int x_, int y_)
			: id(id_), x(x_), y(y_)
		{
		}
		~Player() = default;
	};

	class Manager : public Singleton<Manager> {
	private:
		friend class Singleton<Manager>;
		Manager();
		~Manager();

	private:
		volatile uint32_t _player_id_count = 0; 
		std::queue<RecvChange> _recvStreams; // Network will push to here

		std::unordered_map<unsigned int, std::unique_ptr<Player>> _players; 

	public:
		inline void EnqueueChange(const int type, const unsigned int id,
			const int x, const int y) noexcept {
			_recvStreams.push(RecvChange{ type, id, x, y });
		}
		
		void Update() noexcept; 
		void Render() noexcept; 
	};

}