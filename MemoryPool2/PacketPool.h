#pragma once

#ifdef _DEBUG
#define PACKET_ASSERT(x) if (!(x)) __debugbreak()
#else
#define PACKET_ASSERT(x) ((void)0)
#endif

class Packet { // Serial Buffer 
public:
    static constexpr size_t PACKET_SIZE = 4096; 
	static constexpr size_t BUFFER_SIZE = PACKET_SIZE - sizeof(size_t) * 2; 

private:
	char _buffer[BUFFER_SIZE] = {};
    size_t _head = 0; 
    size_t _tail = 0; 

public:
    Packet() noexcept : _head(0), _tail(0), _buffer{} {}
	~Packet() noexcept = default; 
    
    inline size_t GetUsedSize() const noexcept { return _tail - _head; }
    inline size_t GetFreeSize() const noexcept 
        { return BUFFER_SIZE - GetUsedSize(); }
    void Clear() noexcept { _head = _tail = 0; }

    template<typename T>
    inline Packet& operator<<(const T& v) noexcept {
		// PACKET_ASSERT(_tail + sizeof(T) <= BUFFER_SIZE);
        memcpy(_buffer + _tail, &v, sizeof(T));
        _tail += sizeof(T);
        return *this;
    }

    template<typename T>
    inline Packet& operator>>(T& v) noexcept {
		// PACKET_ASSERT(_head + sizeof(T) <= _tail); 
        memcpy(&v, _buffer + _head, sizeof(T));
        _head += sizeof(T);
        return *this;
    }

    inline Packet& operator<<(const std::string& str) noexcept {
        uint16_t len = str.size();
        // PACKET_ASSERT(_tail + sizeof(len) + len <= BUFFER_SIZE);
        *this << len;
        memcpy(_buffer + _tail, str.data(), len);
        _tail += len;
        return *this;
    }

    inline Packet& operator>>(std::string& str) noexcept {
        uint16_t len;
        *this >> len;
        // PACKET_ASSERT(_head + len <= _tail);
        str.resize(len);
        memcpy((void*)str.data(), _buffer + _head, len);
        _head += len;
        return *this;
    }
};


namespace Win {
class PacketPool {
private:
    struct Node {
        Node* next;
    };

    Node* _freeList = nullptr;
    std::vector<void*> _chunks;

public:
    static PacketPool& Get() {
        thread_local PacketPool instance;
        return instance;
    }

    Packet* Acquire() {
        if (!_freeList) CommitBlocks();
        Node* node = _freeList;
        _freeList = _freeList->next;
        return new(node) Packet();
    }

    void Release(Packet* pkt) {
        Node* node = reinterpret_cast<Node*>(pkt);
        node->next = _freeList;
        _freeList = node;
    }

private:
    void CommitBlocks() {
        static constexpr size_t COUNT = 64;
        void* chunk = VirtualAlloc(nullptr, sizeof(Packet) * COUNT,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        _chunks.push_back(chunk);

        Packet* base = static_cast<Packet*>(chunk);
        for (size_t i = 0; i < COUNT; ++i) {
            Release(base + i);  
        }
    }

    ~PacketPool() {
        for (void* chunk : _chunks) {
            VirtualFree(chunk, 0, MEM_RELEASE);
        }
    }
}; // end of class PacketPool 
}  // namespace Win 
