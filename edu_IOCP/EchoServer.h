#pragma once

class EchoServer : public IOCompletionPort
{
	virtual void OnClientConnect(const UINT32 clientIndex) override {
		fprintf_s(stdout, "[Client %d] Connected.\n", clientIndex);
	}

	virtual void OnClientDiscconnect(const UINT32 clientIndex) override {
		fprintf_s(stdout, "[Client %d] Disconnected.\n", clientIndex);
	}

	virtual void OnClientReceive(const UINT32 clientIndex,
		const UINT32 size, char* pData) override {
		fprintf_s(stdout, "[Client %d] Received %d bytes: %s\n",
			clientIndex, size, pData);
	}
};