#pragma once

#pragma comment(lib, "ws2_32.lib") // Winsock Library
#pragma comment(lib, "winmm.lib")  // TimeBeginPeriod, TimeEndPeriod 
// stdafx.h 

// Upper layer of Windows 
#include <winsock2.h>
#include <ws2tcpip.h>

// Windows API 
#include <windows.h> 

// After Windows.h 
#include <timeapi.h>

// C/C++ Standard Runtime Library System 
#include <atomic>
#include <thread>

// C/C++ Standard Runtime Library 
#include <iostream>
#include <vector>
#include <string> 
#include <queue>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Win32 Multithreading 
#include <process.h> 

// Custom Header Files 
#include "Types.h" 
