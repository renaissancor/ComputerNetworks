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

	engine.Run();
	engine.Shutdown();

    return 0; 
}

/*
accept 당 소켓 하나 ? 
backlog queue 

session 이랑 accept 로 받는 소켓은 별개 
backlog queue 는 TCP session 정보를 들고 있고 
그걸 accept 로 뽑아서 소켓을 설정하는 것 

TCP Session 
Backlog queue refresh timing 


*/