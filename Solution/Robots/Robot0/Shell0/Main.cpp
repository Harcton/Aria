#include <iostream>
#include <functional>
#include <atomic>
#include <Codex/Log.h>
#include <Codex/GPIO.h>
#include <Codex/Servo.h>
#include <Codex/Time.h>
#include <Codex/Codex.h>
#include <Codex/Protocol.h>
#include <Codex/SocketTCP.h>

std::atomic<int> receivedHandlerCallCount(0);
void receiveHandler(const void* data, const size_t bytes)
{
	codex::log::info("Receive handler received " + std::to_string(bytes) + " bytes");
	receivedHandlerCallCount++;
}

int main(int argc, char** argv)
{
	codex::initialize();
	codex::log::info("Shell0 initializing... 2");
	
	/*
	uint64_t someData;
	codex::protocol::WriteBuffer writeBuffer;
	writeBuffer.resize(sizeof(someData));
	writeBuffer.write(&someData, sizeof(someData));

	codex::SocketTCP socketTCP;
	socketTCP.resizeReceiveBuffer(64000);
	if (socketTCP.connect("192.168.10.52", 41523))
		codex::log::info("Successfully connected the tcp socket!");
	if (socketTCP.startReceiving(std::bind(&receiveHandler, std::placeholders::_1, std::placeholders::_2)))
		codex::log::info("TCP socket has began successfully receiving data!");
	if (socketTCP.sendPacket(writeBuffer) == writeBuffer.getSize())
		codex::log::info("TCP socket successfully sent a packet!");

	codex::log::info("Awaiting for a response...");
	while (receivedHandlerCallCount == 0)
	{
		//Blocks
	}
	std::getchar();*/

	//Create servo
	codex::Servo servo;
	servo.setPin(codex::gpio::pin_3);
	servo.mapPosition(codex::time::milliseconds(0), codex::time::milliseconds(2));
	servo.run();
	
	bool run = true;
	while (run)
	{
		int position;
		codex::log::info("Input position\n>");
		std::cin >> position;
		servo.setPosition((unsigned char)position);

		//Stop
		if (position < 0)
		{
			run = false;
			servo.stop();
		}
	}


	codex::uninitialize();
	return 0;
}