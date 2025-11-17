#include "stdafx.h"
#include "engine.h"

// main.cpp 

int main() {
	if (!Engine::GetInstance().Initialize()) {
		fwprintf_s(stderr, L"Engine Initialization Failed!\n");
		return -1; 
	}
	if (!Engine::GetInstance().ConnectToServer()) {
		fwprintf_s(stderr, L"Failed to Connect to Server!\n");
	}
	else {
		Engine::GetInstance().Run();
	}
	Engine::GetInstance().Shutdown();
	return 0; 
}