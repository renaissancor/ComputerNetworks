#include "stdafx.h"
#include "engine.h"
#include "console.h"
#include "networks.h"
#include "keyboard.h" 
#include "gameplay.h"

// engine.cpp 

Engine Engine::instance; 

Engine::Engine()
	: TickCount(0), FPS_prev(0), FPS_curr(0),
	FPS_render_prev(0), FPS_render_curr(0), FPS_updated(0)
{
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&time_init);
}

Engine::~Engine()
{}

bool Engine::Initialize() noexcept {
	if (!Console::Manager::GetInstance().Initialize()) {
		fwprintf_s(stderr, L"Console Initialization Failed!\n");
		return false; 
	}
	if (!Networks::Manager::GetInstance().Initialize()) {
		fwprintf_s(stderr, L"Network Initialization Failed!\n");
		return false; 
	}
	return true; 
}

void Engine::Update() noexcept {
	// To Be Implemented 
	if(TickCount % 3 == 0) Keyboard::Manager::GetInstance().Update(); 
	if (Keyboard::Manager::GetInstance().IsKeyTapp(Keyboard::ESCAPE)) StopRunning();		
	Networks::Manager::GetInstance().Recv();
	Gameplay::Manager::GetInstance().Update(); 
}

void Engine::Render() const noexcept {
	if (TickCount % 4 == 0) Sleep(10); // Simulate Rendering Load 
	Sleep(10);

	Gameplay::Manager::GetInstance().Render(); 
	char buffer[80];
	sprintf_s(buffer, "     Tick : %10d", TickCount);
	Console::Manager::GetInstance().DrawLine(24, 0, buffer);
	sprintf_s(buffer, "FPS (Logic) : %d", FPS_prev);
	Console::Manager::GetInstance().DrawLine(24, 30, buffer);
	sprintf_s(buffer, "FPS (Render) : %d", FPS_render_prev);
	Console::Manager::GetInstance().DrawLine(24, 50, buffer);	
	
	Console::Manager::GetInstance().Display();
}

bool Engine::ConnectToServer( ) noexcept {
	for (int i = 0; i < 5; ++i) {
		if (Networks::Manager::GetInstance().Connect()) {
			fwprintf_s(stdout, L"Connected to Server Successfully!\n");
			return true;
		}
		else {
			fwprintf_s(stderr, L"Connection Attempt %d Failed!\n", i + 1);
			Sleep(1000); 
		}
	}

	fwprintf_s(stdout, L"Press any key to start...\n");
	while (_kbhit()) Sleep(100);
	return false; 
}

void Engine::Run() noexcept {
	const double frame_time = 1000.0 / FRAMES_PER_SECOND;
	double time_start_plan = GetTime();
	double time_limit_plan = time_start_plan + frame_time;
	timeBeginPeriod(1);
	while (InterlockedAdd(&running, 0) != 0) {
		TickCount++;
		time_start_plan = time_limit_plan;
		time_limit_plan += frame_time;
		double time_start_logic = GetTime();

		Update(); 

		FPS_curr++;
		double FPS_after_logic = GetTime();
		if ((FPS_after_logic - FPS_updated) >= 1000) {
			FPS_prev = FPS_curr;
			FPS_curr = 0;
			FPS_render_prev = FPS_render_curr;
			FPS_render_curr = 0;
			FPS_updated += 1000;
		}
		if (FPS_after_logic > time_limit_plan) continue; // Skip Rendering if behind schedule 
		FPS_render_curr++;

		Render();

		double time_final_real = GetTime();
		if (time_final_real < time_limit_plan) {
			Sleep(static_cast<DWORD>(time_limit_plan - time_final_real));
		}
	}
	timeEndPeriod(1);
}

void Engine::Shutdown() noexcept {
	Networks::Manager::GetInstance().Shutdown(); 
	Console::Manager::GetInstance().Shutdown();
}