#include "stdafx.h"
#include "engine.h" 
#include "inputs.h"
#include "console.h" 
#include "Network.h"
#include "Logger.h" 

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
	
	if (!Logger::Manager::GetInstance().Initiate()) {
		fprintf_s(stderr, "Logger Manager Initiation Failed!\n");
		return false;
	}
	if (!Console::Manager::GetInstance().Initiate()) {
		Logger::Manager::GetInstance().Log("Console Manager Initiation Failed!");
		Logger::Manager::GetInstance().Shutdown(); 
		return false;
	}
	if (!Network::Manager::GetInstance().Initiate()) {
		Logger::Manager::GetInstance().Log("Network Manager Initiation Failed!");
		Console::Manager::GetInstance().Shutdown();
		Logger::Manager::GetInstance().Shutdown();
		return false;
	}
	
	hThread = (HANDLE) _beginthreadex(
		nullptr, 
		0, // default stack size 1024 * 1024 
		[](void* param) -> unsigned {
			Engine::GetInstance().RunRecvThread();
			return 0;
		},
		nullptr, 0, nullptr);

	if (hThread == INVALID_HANDLE_VALUE) {
		Console::Manager::GetInstance().Shutdown(); 
		Logger::Manager::GetInstance().Shutdown(); 
		return false; 
	}

	return true; 
}

void Engine::Shutdown() noexcept 
{
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	Network::Manager::GetInstance().Shutdown();
	Console::Manager::GetInstance().Shutdown();
	Logger::Manager::GetInstance().Shutdown(); 
}

void Engine::Update() noexcept 
{
}

void Engine::Render() noexcept 
{
	Console::Manager::GetInstance().draw_line(0, ("Tick : " + std::to_string(TickCount)).c_str()); 
	Console::Manager::GetInstance().draw_line(0, 20, ("FPS (Logic) : " + std::to_string(FPS_prev)).c_str());
	Console::Manager::GetInstance().draw_line(0, 40, ("FPS (Render) : " + std::to_string(FPS_render_prev)).c_str());
	Inputs::Manager::GetInstance().Render(); 
	Logger::Manager::GetInstance().Render(); 
	Console::Manager::GetInstance().Render(); // Render Console 
}

void Engine::RunMainThread() noexcept {
	const double frame_time = 1000.0 / FRAMES_PER_SECOND; 
	double time_start_plan = GetTime(); 
	double time_limit_plan = time_start_plan + frame_time; 

	timeBeginPeriod(1); 
	while (InterlockedAdd(&running, 0) != 0) {
		TickCount++; 
		time_start_plan = time_limit_plan; 
		time_limit_plan += frame_time; 

		double time_start_logic = GetTime(); 
		
		Inputs::Manager::GetInstance().Update();

		FPS_curr++;
		double FPS_after_logic = GetTime(); 
		if((FPS_after_logic - FPS_updated) >= 1000) {
			FPS_prev = FPS_curr;
			FPS_curr = 0;
			FPS_render_prev = FPS_render_curr;
			FPS_render_curr = 0;
			FPS_updated = FPS_after_logic;
		}

		/*
		if (FPS_curr > time_limit_plan) continue; // Skip Render if Logic took too long 
		else FPS_render_curr++;
		*/
		FPS_render_curr++;

		Render(); // Render Console 
		// Sleep(10); // For Test 

		double time_final_real = GetTime(); 
		if (time_final_real < time_limit_plan) {
			Sleep(static_cast<DWORD>(time_limit_plan - time_final_real));
		}
	}
	timeEndPeriod(1); 
}

// Secoundary Thread, Receive from Server, and Log. Log might be third thread in future, but for now it's fine. 
void Engine::RunRecvThread() noexcept { 
	while (InterlockedAdd(&running, 0) != 0) {
		Network::Manager::GetInstance().Update(); 
		Sleep(1);
	}
}
