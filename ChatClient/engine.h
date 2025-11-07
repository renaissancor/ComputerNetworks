#pragma once

class Engine {
private:
	static constexpr const double FRAMES_PER_SECOND = 100.0; 
	static Engine instance;
	Engine();
	~Engine();
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

private:
	volatile LONG running = 2'147'483'647; // For Thread Control 
	alignas(64) 
	int TickCount = 0; 
	int FPS_prev = 0; 
	int FPS_curr = 0; 
	int FPS_render_prev = 0;
	int FPS_render_curr = 0;
	double FPS_updated = 0;

	LARGE_INTEGER time_init; 
	LARGE_INTEGER frequency; 

	// Thread 
	HANDLE hThread = INVALID_HANDLE_VALUE; 

public:
	inline static Engine& GetInstance() noexcept { return instance; }
	inline void StopRunning() noexcept { InterlockedExchange(&running, 0); }
	inline const double GetTime() const noexcept {
		LARGE_INTEGER current;
		QueryPerformanceCounter(&current);
		return (static_cast<double>
			((current.QuadPart - time_init.QuadPart) * 1000))
			/ (static_cast<double>
			(frequency.QuadPart));
	}
public:
	bool Initiate() noexcept;
	void Shutdown() noexcept;
	void Update() noexcept;
	void Render() noexcept;
	void RunMainThread() noexcept; 
	void RunRecvThread() noexcept; 

};