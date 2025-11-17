#pragma once
// stdafx.h 

// Upper layer of Windows 
#include <winsock2.h>
#include <ws2tcpip.h>

// Windows API 
#include <windows.h> 

// After Windows.h 
#include <timeapi.h>

// C/C++ Standard Runtime Library 
#include <cstdio>
#include <cstdlib>
#include <cstring> 
#include <iostream>
#include <vector>
#include <list>
#include <string> 
#include <queue>
#include <set> 
#include <map>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <fstream> 
#include <conio.h>

// _wfsopen_s 
#include <io.h>  // _wsopen_s
#include <fcntl.h> // _O_RDONLY 
#include <share.h> // _SH_DENYNO 

// Win32 Multithreading 
#include <process.h> 

#pragma comment(lib, "ws2_32.lib") // Winsock Library
#pragma comment(lib, "winmm.lib")  // TimeBeginPeriod, TimeEndPeriod 

// Process Data Types 
#include "protocol.h"


