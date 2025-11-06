#include "stdafx.h"
#include "engine.h" 
#include "keyboard.h"
#include "console.h" 
#include "Network.h"

// engine.cpp 

Engine Engine::instance;

Engine::Engine() 
	: TickCount(0), FPS_prev(0), FPS_curr(0), 
	time_init(), frequency() 
{
}

Engine::~Engine() 
{
}

bool Engine::Initiate() noexcept {
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&time_init);
	if (!Console::Manager::GetInstance().Initiate()) return false; 


	return true; 
}

void Engine::Shutdown() noexcept 
{
}

void Engine::Update() noexcept 
{
	Input::Manager::GetInstance().Update(); 
	Network::Manager::GetInstance().Update(); 

}

void Engine::Render() noexcept 
{
	Console::Manager::GetInstance().draw_line(0, ("Tick : " + std::to_string(TickCount)).c_str()); 
	Console::Manager::GetInstance().draw_line(0, 20, ("FPS (Logic) : " + std::to_string(FPS_prev)).c_str());
	Network::Manager::GetInstance().Render(); 
	Input::Manager::GetInstance().Render(); 
	Console::Manager::GetInstance().Render(); // Render Console 
}

void Engine::Run() noexcept {
	const double frame_time = 1000.0 / FRAMES_PER_SECOND; 
	double time_start_plan = GetTime(); 
	double time_limit_plan = time_start_plan + frame_time; 

	timeBeginPeriod(1); 
	for (;; TickCount++) {
		time_start_plan = time_limit_plan; 
		time_limit_plan += frame_time; 

		Update(); // Update Logic 

		FPS_curr++;
		double FPS_after_logic = GetTime(); 
		if((FPS_after_logic - FPS_updated) >= 1000) {
			FPS_prev = FPS_curr;
			FPS_curr = 0;
			FPS_render_prev = FPS_render_curr;
			FPS_render_curr = 0;
			FPS_updated = FPS_after_logic;
		}

		if (FPS_curr > time_limit_plan) continue;
		else FPS_render_curr++;

		Render(); // Render Console 
		Sleep(1); 

		double time_final_real = GetTime(); 
		if (time_final_real < time_limit_plan) {
			Sleep(static_cast<DWORD>(time_limit_plan - time_final_real));
		}
	}
	timeEndPeriod(1); 
}

