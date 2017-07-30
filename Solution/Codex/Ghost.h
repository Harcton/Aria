#pragma once

/**
	Interface to be used in the Aria system.
*/
class Ghost
{
public:
	Ghost();
	~Ghost();

	void update();

	void sendPacketUDP(const unsigned char* buffer, const size_t bytes);
	void sendPacketTCP(const unsigned char* buffer, const size_t bytes);
	void receiveHandler(const unsigned char* buffer, const size_t bytes);

};

