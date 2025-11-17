#pragma once

class Engine {
private:
	static constexpr const double FRAMES_PER_SECOND = 120.0; 
	Engine();
	~Engine();
	Engine(Engine const&) = delete;
	Engine const& operator=(Engine const&) = delete;
	static Engine instance; 
private:
	volatile LONG running = 2'147'483'647;
	alignas(64)
	int TickCount = 0;
	int FPS_prev = 0;
	int FPS_curr = 0;
	int FPS_render_prev = 0;
	int FPS_render_curr = 0;
	double FPS_updated = 0;

	LARGE_INTEGER time_init;
	LARGE_INTEGER frequency;
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
	bool Initialize() noexcept;
	void Shutdown() noexcept;
	bool ConnectToServer() noexcept; 
	void Run() noexcept;
	void Update() noexcept;
	void Render() const noexcept;
};