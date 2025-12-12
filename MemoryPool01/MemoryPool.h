#pragma once

namespace Win {

template<typename T> 
class MemoryPool {
private:
	
	struct Node {
		T instance; // something large 
		Node* next;
	};

	int _capacity; 
	int _useCount; 
	const bool _callPlacementNew; 

	Node* _head; 
	void* _ptrVirtualAlloc; 

public:
	inline int GetCapacity() const { return _capacity; }
	inline int GetUseCount() const { return _useCount; }

	MemoryPool(int blockCount, bool callPlacementNew = true);
	virtual ~MemoryPool(); 

	T* Alloc(); 
	bool Free(T* ptr); 
};

} // end of namespace Win 

template<typename T>
Win::MemoryPool<T>::MemoryPool(int blockCount, bool callPlacementNew)
: _capacity(blockCount), _useCount(0), _callPlacementNew(callPlacementNew) {
	static_assert(offsetof(Node, instance) == 0);

	int sizeMemoryPool = blockCount * sizeof(Node); 

	_ptrVirtualAlloc = VirtualAlloc(
		nullptr, 
		(SIZE_T) sizeMemoryPool,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_READWRITE
	); 

	if (_ptrVirtualAlloc == nullptr) throw std::bad_alloc(); 

	Node* itNext = nullptr; 
	Node* itCurr = reinterpret_cast<Node*>(_ptrVirtualAlloc);
	for (int i = 0; i < blockCount; ++i) {
		itCurr->next = itNext; 
		itNext = itCurr; 
		itCurr += 1;
	}
	_head = itNext;
}

template<typename T>
Win::MemoryPool<T>::~MemoryPool() {

	VirtualFree(
		_ptrVirtualAlloc, 
		0,
		MEM_RELEASE
	);

}

template<typename T>
T* Win::MemoryPool<T>::Alloc() { 
	T* pAlloc = reinterpret_cast<T*>(_head);
	if (pAlloc == nullptr) return nullptr; 
	_head = _head->next; 
	if(_callPlacementNew) new (pAlloc) T(); // Placement New 
	++_useCount;
	return pAlloc; 
}

template<typename T>
bool Win::MemoryPool<T>::Free(T* pFreed) {
	// Think about wrong pointer return next class 
	if (pFreed == nullptr) return false; 
	// Implement pointer validation check later. 
	Node* pNode = reinterpret_cast<Node*>(pFreed);
	if (_callPlacementNew) pFreed->~T();

	pNode->next = _head; 
	_head = pNode; 
	--_useCount;
	return true; 
}


/*
Template, WinAPI VirtualAlloc based Memory Pool 
Designed for classes that require instance generator / terminator function 
Also, class that call dynamic allocation inside class generator is not 
designed to use this memory pool structure. 
For instance, Session class with RingBuffer dynamic allocation should 
NOT call placement new and terminator function. 
*/


