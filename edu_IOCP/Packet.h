#pragma once

struct PacketData 
{
	unsigned int SessionIndex = 0; 
	unsigned int DataSize = 0; 
	char* pPacketData = nullptr; 

	void Set(PacketData& other) {
		SessionIndex = other.SessionIndex;
		DataSize = other.DataSize;
		pPacketData = new char[other.DataSize];
		CopyMemory(pPacketData, other.pPacketData, other.DataSize); 
		// memcpy_s(pPacketData, other.DataSize, other.pPacketData, other.DataSize);
	}

	void Release() {
		if (pPacketData != nullptr) {
			delete[] pPacketData; 
			pPacketData = nullptr; 
		}
	}

};