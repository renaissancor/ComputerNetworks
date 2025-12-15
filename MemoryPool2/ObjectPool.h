#pragma once

// ObjectPool.h

namespace Win {
    template<typename T>
    class ObjectPool {
    private:
        static constexpr uint64_t GUARD_PATTERN = 0xAABBCCDDAABBCCDD;
        static constexpr uint64_t FREED_PATTERN = 0xFEFEFEFEFEFEFEFE;

        struct Node {
            uint64_t guardHead;
            Node* next;
            alignas(T) unsigned char storage[sizeof(T)];
            uint64_t guardTail;
        };

        Node* _freeList = nullptr;
        size_t _capacity = 0;
        size_t _useCount = 0;

    public:
        ObjectPool() = default;
        ~ObjectPool();
        ObjectPool(const ObjectPool&) = delete;
        ObjectPool& operator=(const ObjectPool&) = delete;

        inline size_t GetCapacity() const { return _capacity; }
        inline size_t GetUseCount() const { return _useCount; }

        T* Acquire();
        void Release(T* obj);
    };
}  // namespace Win


template<typename T>
Win::ObjectPool<T>::~ObjectPool() {
    if (_useCount != 0) {
        DebugBreak();  // Memory leak detected
	}

    while (_freeList) {
        Node* next = _freeList->next;
        delete _freeList;
        _freeList = next;
    }
}

template<typename T>
T* Win::ObjectPool<T>::Acquire() {
    Node* node;

    if (_freeList) {
        node = _freeList;
        _freeList = _freeList->next;
    }
    else {
        node = new Node();
        node->guardTail = GUARD_PATTERN;
        ++_capacity;
    }

    node->guardHead = GUARD_PATTERN;
    node->next = nullptr;
    ++_useCount;

    return new(node->storage) T();
}

template<typename T>
void Win::ObjectPool<T>::Release(T* obj) {
    if (obj == nullptr) return;

    Node* node = reinterpret_cast<Node*>(
        reinterpret_cast<unsigned char*>(obj) - offsetof(Node, storage)
        );

    if (node->guardHead == FREED_PATTERN) {
        DebugBreak();  // Double free
        return;
    }

    if (node->guardHead != GUARD_PATTERN ||
        node->guardTail != GUARD_PATTERN) {
        DebugBreak();  // Corruption
        return;
    }

    obj->~T();
    node->guardHead = FREED_PATTERN;
    node->next = _freeList;
    _freeList = node;
    --_useCount;
}
