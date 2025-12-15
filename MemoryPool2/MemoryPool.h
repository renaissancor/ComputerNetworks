#pragma once

// MemoryPool.h 

namespace Win {
class free_singly_linked_list {
private:
	struct block_node {
		block_node* next;
	};
	using block = block_node; 
	
	block* _head = nullptr;
	int _blockSize = 0; 

public:
	void* acquire() { // pop a block 
		if (_head == nullptr) return nullptr;
		block* node = _head;
		_head = _head->next; 
		return reinterpret_cast<void*>(node); 
	}
	void release(void* ptr) { // push a block 
		block* node = reinterpret_cast<block*>(ptr);
		node->next = _head; 
		_head = node;
	}
}; // end of class FreeList 
using free_list = free_singly_linked_list; 

class MemoryPool {
private:
	static constexpr int ClassCount = 8;
	static constexpr size_t ClassSizes[ClassCount] = {
		32, 64, 128, 256, 512, 1024, 2048, 4096
	};

	struct SizeClass {
		size_t blockSize; 
		size_t stride; 
		free_list freeList;
		unsigned char* base; 
		size_t committedSize;
		size_t reservedSize;
	};

	SizeClass _classes[ClassCount] = {}; // 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 bytes

	void CommitBlocks(SizeClass& sc); 

public:
	MemoryPool();
	~MemoryPool();
	void* Acquire(size_t size);
	void  Release(void* ptr, size_t size);



}; // end of class MemoryPool 
} // end of namespace Win 

void Win::MemoryPool::CommitBlocks(SizeClass& sc) {
	// Implementation of committing more blocks to the size class
	// This is a placeholder; actual implementation would use VirtualAlloc
	const size_t commitSize = 64 * sc.stride; // Commit 64 blocks at a time
	unsigned char* newBlock = reinterpret_cast<unsigned char*>(
		VirtualAlloc(
		nullptr,
		commitSize,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE
	));
	
	int blockCount = commitSize / sc.stride; 
	for (int i = 0; i < blockCount; ++i) {
		sc.freeList.release(newBlock + i * sc.stride); 
	}
}

Win::MemoryPool::MemoryPool() {
	for (int i = 0; i < ClassCount; ++i) {
		SizeClass& sc = _classes[i];
		sc.blockSize = ClassSizes[i];
		sc.stride = ClassSizes[i];
		sc.freeList = free_list();
		sc.base = nullptr;
		sc.committedSize = 0;
		sc.reservedSize = 0;
	}
}

Win::MemoryPool::~MemoryPool() {
	for (int i = 0; i < ClassCount; ++i) {
		SizeClass& sc = _classes[i];
		if (sc.base != nullptr) {
			VirtualFree(sc.base, 0, MEM_RELEASE);
			sc.base = nullptr;
			sc.committedSize = 0;
			sc.reservedSize = 0;
		}
	}
}

void* Win::MemoryPool::Acquire(size_t size) {
	for (int i = 0; i < ClassCount; ++i) {
		SizeClass& sc = _classes[i];
		if (size <= sc.blockSize) {
			void* ptr = sc.freeList.acquire();
			if (ptr == nullptr) {
				CommitBlocks(sc);
				ptr = sc.freeList.acquire();
			}
			return ptr;
		}
	}
	// For sizes larger than the largest class, use VirtualAlloc directly
	return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void Win::MemoryPool::Release(void* ptr, size_t size) {
	for (int i = 0; i < ClassCount; ++i) {
		SizeClass& sc = _classes[i];
		if (size <= sc.blockSize) {
			sc.freeList.release(ptr);
			return;
		}
	}
	VirtualFree(ptr, 0, MEM_RELEASE);
}


/*
Use instead of malloc/free 
Have FreeList of free blocks 
sized 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 bytes
Win::Alloc(size_t size); 
Win::Free(void* ptr); 
*/
