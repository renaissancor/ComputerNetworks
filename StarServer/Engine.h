#pragma once

// Engine.h 

class Engine : public Singleton<Engine> {
private:
	friend class Singleton<Engine>;
	Engine();
	~Engine();
private:
	constexpr static const double FRAMES_PER_SECOND = 60.0;

	volatile LONG _running = 1;

	uint64_t _TickCount = 0;
	uint32_t _FPS_prev = 0;
	uint32_t _FPS_curr = 0;
	double _FPS_updated = 0; 

	LARGE_INTEGER _frequency;
	LARGE_INTEGER _time_init;
public:
	inline void StopRunning() noexcept { InterlockedExchange(&_running, 0); }
	inline const long long GetTimeLL() const noexcept {
		LARGE_INTEGER current;
		QueryPerformanceCounter(&current);
		return current.QuadPart; 
	}
	inline const long long GetFrequencyLL() const noexcept 
	{ return _frequency.QuadPart; }

	inline const double GetTimeD() const noexcept {
		LARGE_INTEGER current;
		QueryPerformanceCounter(&current);
		return (static_cast<double>
			((current.QuadPart - _time_init.QuadPart) * 1000))
			/ (static_cast<double>
			(_frequency.QuadPart));
	}

	bool Initialize() noexcept;
	void Shutdown() noexcept;
	void Run() noexcept;
	void Update() noexcept;
	void Render() const noexcept;
};