#pragma once
class Shell
{
public:
	Shell();
	~Shell();

	virtual void update();

	void sendPacketUDP(const unsigned char* buffer, const size_t bytes);
	void sendPacketTCP(const unsigned char* buffer, const size_t bytes);
	void receiveHandler(const unsigned char* buffer, const size_t bytes);
};

