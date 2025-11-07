#include "stdafx.h"
#include "engine.h" 

// main.cpp 

int main()
{
	Engine& engine = Engine::GetInstance(); 
	if (!engine.Initiate()) {
		fprintf_s(stderr, "Engine Initiation Failed!\n");
		return -1;
	}

	engine.RunMainThread();
	engine.Shutdown();

    return 0; 
}
