#pragma once

// Upper layer of Windows 
#include <winsock2.h>
#include <ws2tcpip.h>

// Windows API 
#include <windows.h> 

// After Windows.h 
#include <timeapi.h>

// C/C++ Standard Runtime Library 
#include <iostream>
#include <vector>
#include <string> 
#include <queue>

// Win32 Multithreading 
#include <process.h> 

#pragma comment(lib, "ws2_32.lib") // Winsock Library
#pragma comment(lib, "winmm.lib")  // TimeBeginPeriod, TimeEndPeriod 
