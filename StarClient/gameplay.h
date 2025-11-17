#pragma once

namespace Gameplay {
	constexpr const int GAME_WIDTH  = 80;
	constexpr const int GAME_HEIGHT = 23;

	using std::unique_ptr; 
	using std::list; 
	using std::queue; 
	using std::make_unique; 
	using std::unordered_map;

	struct Player {
		unsigned int id = 0; 
		int x = 0;
		int y = 0; 
		Player(unsigned int id_, int x_, int y_) 
			: id(id_), x(x_), y(y_) 
		{}
		~Player() = default;
	};

	struct Change {
		enum Type {
			LOGIN,
			CREATE,
			REMOVE,
			MOVE
		} type;
		unsigned int id = 0; 
		int x = 0; 
		int y = 0; 
		Change(Type type_, unsigned int id_, int x_, int y_) 
			: type(type_), id(id_), x(x_), y(y_) 
		{}
	};

	class Manager {
	private:
		Manager();
		~Manager();
		Manager(Manager const&) = delete;
		Manager const& operator=(Manager const&) = delete; 
		static Manager instance;
	private:
		unsigned int _localPlayerID = 0; 
		list<unique_ptr<Player>> _players; 
		queue<Change> _changes; 
		unordered_map<unsigned int, Player> _playerMap;

	public:
		inline static Manager& GetInstance() noexcept { return instance; }
		inline void EnqueueChange(int type, unsigned int id, int x, int y) noexcept {
			_changes.push(Change(static_cast<Change::Type>(type), id, x, y));
		}
		inline const Player* GetLocalPlayer() const noexcept {
			for (const unique_ptr<Player>& player : _players) {
				if (player->id == _localPlayerID) return player.get();
			}
			return nullptr;
		}
		inline void SetLocalPlayerID(unsigned int id) noexcept {
			_localPlayerID = id; 
		}

		void MoveLocalPlayerBy(const int dx, const int dy) noexcept; 

		void Initialize() noexcept; 
		void Update() noexcept;
		void Render() const noexcept; 
	};
}