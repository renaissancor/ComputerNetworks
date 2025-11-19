#include "stdafx.h"
#include "Engine.h" 

// main.cpp 

int main(int argc, char* argv[]) 
{
	if (!Engine::GetInstance().Initialize()) {
		return -1; 
	}
	Engine::GetInstance().Run();
	Engine::GetInstance().Shutdown();
	return 0; 
}