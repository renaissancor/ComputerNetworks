#include "stdafx.h"
#include "ObjectPool.h"

// TestObjectPool.cpp 

void TestObjectPool() {
	Win::ObjectPool<std::string> stringPool;
	std::string* str1 = stringPool.Acquire();
	*str1 = "Hello, ";
	std::string* str2 = stringPool.Acquire();
	*str2 = "World!";
	
	std::cout << *str1 << *str2 << std::endl;
	stringPool.Release(str1);
	stringPool.Release(str2);
}
