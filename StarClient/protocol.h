#pragma once

// protocol.h 

namespace Protocol {
#pragma pack(push, 1)
	struct Packet {
		uint32_t type;
		uint32_t id;
		uint32_t x;
		uint32_t y;
	};
#pragma pack(pop) 

#pragma pack(push, 1)
	struct Packet0 {
		uint32_t type;
		uint32_t id;
		uint32_t x;
		uint32_t y;
	};
#pragma pack(pop) 

#pragma pack(push, 1)
	struct Packet1 {
		uint32_t type;
		uint32_t id;
		uint32_t x;
		uint32_t y;
	};
#pragma pack(pop) 

#pragma pack(push, 1)
	struct Packet2 {
		uint32_t type;
		uint32_t id;
		uint32_t x;
		uint32_t y;
	};
#pragma pack(pop) 

#pragma pack(push, 1)
	struct Packet3 {
		uint32_t type;
		uint32_t id;
		uint32_t x;
		uint32_t y;
	};
#pragma pack(pop) 


}