#include "stdafx.h"
#include "Engine.h"
#include "Console.h"
#include "Network.h"
#include "Game.h"

Engine::Engine()
{
	QueryPerformanceFrequency(&_frequency);
	QueryPerformanceCounter(&_time_init);
}

Engine::~Engine() = default; 

bool Engine::Initialize() noexcept {
	if (!Console::Manager::GetInstance().Initialize()) {
		return false; 
	}
	if (!Network::Manager::GetInstance().Initialize()) {
		Console::Manager::GetInstance().Shutdown(); 
		return false; 
	}
	return true; 
}

void Engine::Update() noexcept {
	Network::Manager::GetInstance().RecvStream(); 
	Game::Manager::GetInstance().Update(); 
	Network::Manager::GetInstance().SendStream(); 
}

void Engine::Render() const noexcept {
	Game::Manager::GetInstance().Render();
	char buffer[80];
	sprintf_s(buffer, "     Tick : %lld", _TickCount);
	Console::Manager::GetInstance().DrawLine(24, 0, buffer);
	sprintf_s(buffer, "FPS (Logic) : %u", _FPS_prev);
	Console::Manager::GetInstance().DrawLine(24, 30, buffer);
	Console::Manager::GetInstance().Display();
}

void Engine::Run() noexcept {
	_running = 1;
	const long long frame_interval = GetFrequencyLL() / FRAMES_PER_SECOND;
	long long time_limit_plan = GetTimeLL() + frame_interval;
	
	const long long fps_interval = GetFrequencyLL();
	long long fps_updated = GetTimeLL();

	timeBeginPeriod(1);

	while (InterlockedAdd(&_running, 0) != 0) {
		// long long time_start_logic = GetTimeLL(); 

		Update(); 
		_TickCount++; 
		_FPS_curr++; 

		long long time_after_logic = GetTimeLL(); 

		if (time_after_logic - fps_updated >= fps_interval) {
			_FPS_prev = _FPS_curr;
			_FPS_curr = 0;
			fps_updated += fps_interval;
		} 

		if (time_after_logic > time_limit_plan) {
			while (time_limit_plan <= time_after_logic) {
				time_limit_plan += frame_interval;
			}
			continue; // Skip Rendering if behind schedule 
		}

		time_limit_plan += frame_interval; 

		// Render Logic Here for Debug Purpose 
		Render(); 

		long long time_final = GetTimeLL();
		if (time_final < time_limit_plan) {
			long long sleep_time_ms = (time_limit_plan - time_final) * 1000 / fps_interval; 
			if (sleep_time_ms > 0) Sleep(static_cast<DWORD>(sleep_time_ms));
		}
	}
	timeEndPeriod(1); 
}

void Engine::Shutdown() noexcept {
	Network::Manager::GetInstance().Shutdown(); 
	Console::Manager::GetInstance().Shutdown();
}