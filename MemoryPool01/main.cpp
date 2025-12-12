#include "stdafx.h"
#include "MemoryPool.h"
#include <iostream>

using namespace Win;

struct DebugObj {
    int x;
    DebugObj() {
        x = 777;
        std::cout << "[DebugObj] Constructor called\n";
    }
    ~DebugObj() {
        std::cout << "[DebugObj] Destructor called\n";
    }
};

void test1() {
    std::cout << "=== MemoryPool Test ===\n";

    MemoryPool<DebugObj> pool(3, true);

    std::cout << "\n-- Alloc() 3 times --\n";
    DebugObj* a = pool.Alloc();
    DebugObj* b = pool.Alloc();
    DebugObj* c = pool.Alloc();

    std::cout << "Values: " << a->x << ", " << b->x << ", " << c->x << "\n";

    std::cout << "\n-- Free() 3 times --\n";
    pool.Free(a);
    pool.Free(b);
    pool.Free(c);

    std::cout << "\n-- Alloc() again (should reuse freed blocks) --\n";
    DebugObj* d = pool.Alloc();
    DebugObj* e = pool.Alloc();
    DebugObj* f = pool.Alloc();

    std::cout << "Values again: " << d->x << ", " << e->x << ", " << f->x << "\n";

    std::cout << "\n-- DONE --\n";
}

struct PODStruct {
    int a;
    int b;
};

void test2() {
    {
        std::cout << "\n=== Test 2: callPlacementNew = false ===\n";

        MemoryPool<PODStruct> pool(5, false);

        PODStruct* p1 = pool.Alloc();
        PODStruct* p2 = pool.Alloc();

        p1->a = 10; p1->b = 20;
        p2->a = 30; p2->b = 40;

        std::cout << p1->a << ", " << p1->b << "\n";
        std::cout << p2->a << ", " << p2->b << "\n";

        pool.Free(p1);
        pool.Free(p2);

        PODStruct* p3 = pool.Alloc();
        PODStruct* p4 = pool.Alloc();

        std::cout << "Reused values: " << p3->a << ", " << p3->b << "\n";
        std::cout << "Reused values: " << p4->a << ", " << p4->b << "\n";
    }
}

void test3() {
    std::cout << "\n=== Test 3: Contiguous memory check ===\n";

    MemoryPool<int> pool(5, false);

    int* p1 = pool.Alloc();
    int* p2 = pool.Alloc();
    int* p3 = pool.Alloc();

    std::cout << "Address p1: " << p1 << "\n";
    std::cout << "Address p2: " << p2 << "\n";
    std::cout << "Address p3: " << p3 << "\n";

    std::cout << "Difference p2 - p1: " << ((char*)p2 - (char*)p1) << " bytes\n";
    std::cout << "Difference p3 - p2: " << ((char*)p3 - (char*)p2) << " bytes\n";
}

void test4() {
    std::cout << "\n=== Test 4: Capacity overflow ===\n";

    MemoryPool<int> pool(2, false);

    int* a = pool.Alloc();
    int* b = pool.Alloc();
    int* c = pool.Alloc(); // capacity ÃÊ°ú ¡æ nullptr expected

    std::cout << "a: " << a << "\n";
    std::cout << "b: " << b << "\n";
    std::cout << "c (should be nullptr): " << c << "\n";
}

static void test5() {
    std::cout << "\n=== Test 5: Stress test (1,000,000 alloc/free) ===\n";

    MemoryPool<int> pool(1000000, false);

    for (int i = 0; i < 1000000; ++i) {
        int* p = pool.Alloc();
        pool.Free(p);
    }

    std::cout << "Stress test completed.\n";
}



int main()
{
    test1();
    test2(); 
    test3();
    test4(); 
    test5(); 

    return 0;
}
