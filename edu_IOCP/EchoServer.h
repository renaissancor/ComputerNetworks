#pragma once

#include "IOCPServer.h"

class EchoServer : public IOCompletionPort
{
	virtual void OnClientConnect(const uint32_t clientIndex) override {
		fprintf_s(stdout, "[Client %d] Connected.\n", clientIndex);
	}

	virtual void OnClientDisconnect(const uint32_t clientIndex) override {
		fprintf_s(stdout, "[Client %d] Disconnected.\n", clientIndex);
	}

	virtual void OnClientReceive(const uint32_t clientIndex,
		const uint32_t size, char* pData) override {
		fprintf_s(stdout, "[Client %d] Received %d bytes: %s\n",
			clientIndex, size, pData);
	}
};