#include <iostream>
#include <functional>
#include <atomic>
#include <Codex/Log.h>
#include <Codex/GPIO.h>
#include <Codex/Servo.h>
#include <Codex/DCMotor.h>
#include <Codex/CodexTime.h>
#include <Codex/Codex.h>
#include <Codex/Protocol.h>
#include <Codex/SocketTCP.h>
#include <Codex/IOService.h>
#include <bcm2835.h>



bool clientReceiveHandler(codex::protocol::ReadBuffer& buffer)
{
	codex::log::info("Client receive handler received " + std::to_string(buffer.getCapacity()) + " bytes");
	for (size_t i = 0; i < buffer.getWrittenSize(); i++)
	{
		uint8_t byte;
		buffer.read(&byte, sizeof(byte));
	}
	return true;
}


int main(int argc, char** argv)
{
	codex::initialize();
	codex::log::info("Shell0 initializing... 10");
	



	//Buffer
	uint64_t someData = 0x0123456789ABCDEF;
	codex::protocol::WriteBuffer writeBuffer(codex::protocol::Endianness::inverted);
	writeBuffer.write((void*)&someData, sizeof(someData));
	writeBuffer.write((void*)&someData, sizeof(someData));

	////Shell socket
	codex::IOService ioservice;
	codex::SocketTCP shellSocket(ioservice);
	shellSocket.resizeReceiveBuffer(64000);
	if (shellSocket.connect("192.168.10.52", 41623))
		codex::log::info("Successfully connected the tcp socket!");
	if (shellSocket.startReceiving(std::bind(&clientReceiveHandler, std::placeholders::_1)))
		codex::log::info("TCP socket has began successfully receiving data!");
	if (shellSocket.sendPacket(writeBuffer))
		codex::log::info("Testing: TCP socket successfully sent a packet!");
	//Loop
	while (true)
	{
		//Blocks
	}



	{
		codex::DCMotor dcMotor;
		dcMotor.setPins(codex::gpio::pin_5, codex::gpio::pin_13, codex::gpio::pin_11);
		dcMotor.run();
		bool run = true;
		while (run)
		{
			int position;
			codex::log::info("Input position\n>");
			std::cin >> position;

			//Stop
			if (position < 0)
			{
				run = false;
			}
		}
		dcMotor.stop();
	}

	/*
	//Just some example code to get things started...
	codex::gpio::Pin aPulseWidth = codex::gpio::pin_5;
	codex::gpio::Pin aInput1 = codex::gpio::pin_13;
	codex::gpio::Pin aInput2 = codex::gpio::pin_11;
	bcm2835_gpio_fsel(aPulseWidth, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(aInput1, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(aInput2, BCM2835_GPIO_FSEL_OUTP);
	codex::gpio::disable(aInput1);
	codex::gpio::enable(aInput2);
	const codex::time::TimeType pulseInterval = codex::time::nanoseconds(2000000);
	int direction = 1;
	codex::time::TimeType data = 0;
	while (1)
	{
		if (data <= 0)
			direction = 1000;
		else if (data >= pulseInterval)
			direction = -1000;
		data += direction;
		codex::gpio::enable(aPulseWidth);
		codex::time::delay(data);
		codex::gpio::disable(aPulseWidth);
		codex::time::delay(pulseInterval - data);
	}
	*/

	/*
	//Create servo
	codex::Servo servo;
	servo.setPin(codex::gpio::pin_3);
	servo.mapPosition(codex::time::milliseconds(0.7f), codex::time::milliseconds(1.75f));
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
	*/

	/*
	codex::gpio::Pin trigPin = codex::gpio::pin_13;
	codex::gpio::Pin echoPin = codex::gpio::pin_12;

	bcm2835_gpio_fsel(trigPin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(echoPin, BCM2835_GPIO_FSEL_INPT);

	codex::gpio::enable(trigPin);
	codex::time::delay(codex::time::seconds(1));
	while(1)
	{
		codex::gpio::disable(trigPin);
		codex::time::delay(codex::time::milliseconds(2));
		codex::gpio::enable(trigPin);
		codex::time::delay(codex::time::milliseconds(10));
		codex::gpio::disable(trigPin);

		while (codex::gpio::read(echoPin) == codex::gpio::PinState::high)
		{
			codex::log::info("loop1");
			codex::time::delay(codex::time::seconds(1));
			//Blocks until pin is low
		}
		while (codex::gpio::read(echoPin) == codex::gpio::PinState::low)
		{
			codex::log::info("loop2");
			codex::time::delay(codex::time::seconds(1));
			//Blocks until pin is high
		}
		codex::time::TimeType duration = 0;
		do
		{
			codex::log::info("loop3");
			codex::time::delay(codex::time::seconds(1));
			static const codex::time::TimeType precision = codex::time::nanoseconds(1000);
			codex::time::delay(precision);
			duration += precision;
		} while (codex::gpio::read(echoPin) == codex::gpio::PinState::high);

		const long distance = (duration / 2) / 29.1;

		codex::log::info("distance: " + std::to_string(distance));

		codex::time::delay(codex::time::milliseconds(500));
	}
	*/

	/*
	uint64_t someData;
	codex::protocol::WriteBuffer writeBuffer;
	writeBuffer.resize(sizeof(someData));
	writeBuffer.write(&someData, sizeof(someData));

	codex::SocketTCP socketTCP;
	socketTCP.resizeReceiveBuffer(64000);
	if (socketTCP.connect("192.168.10.52", 41623))
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
	std::getchar();
	*/
	
	/*
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
	*/

	
	codex::uninitialize();
	return 0;
}