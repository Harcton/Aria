#include <string>
#include <thread>
#include <iostream>
#include <functional>
#include <unordered_map>
#include <atomic>
#include <bcm2835.h>
#include <Codex/Log.h>
#include <Codex/GPIO.h>
#include <Codex/CodexTime.h>
#include <Codex/Codex.h>

#include <Codex/Servo.h>
#include <Codex/DCMotorController.h>
#include <Codex/Protocol.h>
#include <Codex/Display4x8.h>
#include <Codex/SocketTCP.h>
#include <Codex/IOService.h>

bool receiveHandler(codex::protocol::ReadBuffer& buffer)
{
	//Print out buffer contents...
	std::string str;
	buffer.read(str);
	codex::log::info("Received packet (str): " + str);
	return true;
}
int shell(codex::SocketTCP& socket)
{
	codex::log::info("Shell program running...");

	//Socket test
	socket.startReceiving(std::bind(receiveHandler, std::placeholders::_1));
	codex::protocol::WriteBuffer buffer;
	buffer.write(std::string("Yo world!1"));	
	socket.sendPacket(buffer);

	while (1)
	{
		//...
		if (!socket.isConnected())
		{
			codex::log::info("Ghost disconnected.");
			break;
		}
	}
	return 0;
}

std::atomic<bool> onAcceptEnded(false);
bool onAccept(codex::SocketTCP& socket)
{
	if (socket.isConnected())
	{
		codex::log::info("Remote connection accepted.");
		shell(socket);
		socket.disconnect();
	}
	else
	{
		codex::log::info("Failed to accept remote connection.");
	}
	onAcceptEnded = true;
	return false;
}




/*
	Arguments:
	0: path
	1: connect address
	2. connect port

	or

	0: path
	1. receive port
*/
int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);
	codex::log::info("Shell0 initializing...");
	
	//codex::DCMotorController motor;
	//motor.setPins(codex::gpio::pin_11, codex::gpio::pin_13, codex::gpio::pin_15);
	//motor.start();
	//float strength = 0.0f;
	//while (1)
	//{
	//	codex::log::info("Set strength [0, 255] >");
	//	std::cin >> strength;
	//	motor.setStrength(strength / 255.0f);
	//	if (strength < 0.0f)
	//		break;
	//}
	//motor.stop();
	//return 0;

	//Socket
	codex::IOService ioservice;
	codex::SocketTCP socket(ioservice);
	socket.resizeReceiveBuffer(64000);
	
	//Determine initial course of action based on given command line arguments
	if (argc == 2)
	{//Remain waiting

		//Local endpoint
		const int port = std::atoi(argv[1]);
		if (port < 0 || port > std::numeric_limits<uint16_t>::max())
		{
			codex::log::error("Provided server port is invalid: " + std::string(argv[1]));
			return 1;
		}

		codex::log::info("No server endpoint provided in the command line arguments. Awaiting for the ghost to connect to local endpoint at: " + std::to_string(port));
		socket.startAccepting(port, std::bind(&onAccept, std::placeholders::_1));
		while (!onAcceptEnded)
		{/* Blocks until onAccept has ended. */}
	}
	else if (argc == 3)
	{//Connect
		
		//Remote endpoint
		const std::string address = argv[1];
		const int port = std::atoi(argv[2]);

		//Some endpoint validation...
		int periodCount = 0;
		for (size_t i = 0; i < address.size(); i++)
		{
			if (address[i] == '.')
				periodCount++;
		}
		if (periodCount != 3 || address.size() > 15)
		{
			codex::log::error("Provided server address is invalid: " + address);
			return 1;
		}
		if (port < std::numeric_limits<uint16_t>::min() || port > std::numeric_limits<uint16_t>::max())
		{
			codex::log::error("Provided server port is invalid: " + std::to_string(port));
			return 1;
		}
		
		////Try to connect
		//codex::log::info("Connecting to the server at " + address + ", port: " + std::to_string(port));
		//if (socket.connect(address.c_str(), port))
		//{//Connected

		//	codex::log::info("Successfully connected to the server!");
		//	codex::log::info("Requesting ghost...");
		//	if (socket.requestGhost("Ghost0"))
		//	{
		//		codex::log::info("Ghost retrieved! Starting the shell program...");
		//		const int result = shell(socket);
		//		socket.disconnect();
		//		return result;
		//	}
		//	else
		//	{
		//		codex::log::error("Could not retrieve ghost!");
		//		return 1;
		//	}
		//}
		//else
		//{//Failed to connect
		//	codex::log::error("Failed to connect to the provided server endpoint! Press enter to exit the shell...");
		//	return 1;
		//}
	}
	else
	{//Incorrect command line argument usage
		codex::log::error("Incorrect usage of command line arguments! Correct usage: [1]ghost endpoint's address [2]ghost endpoint's port");
		return 1;
	}
	





	//{
	//	codex::DCMotor dcMotor;
	//	dcMotor.setPins(codex::gpio::pin_5, codex::gpio::pin_13, codex::gpio::pin_11);
	//	dcMotor.run();
	//	bool run = true;
	//	while (run)
	//	{
	//		int position;
	//		codex::log::info("Input position\n>");
	//		std::cin >> position;

	//		//Stop
	//		if (position < 0)
	//		{
	//			run = false;
	//		}
	//	}
	//	dcMotor.stop();
	//}
	

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


/*
//TEST BASE

std::atomic<int> strength(0);
std::atomic<bool> keepRunningInput(false);
void runInput()
{
keepRunningInput = true;
while (keepRunningInput)
{
codex::log::info("Input strength:\n>");
int val;
std::cin >> val;
strength = val;
}
}

//Main
std::thread inputThread(runInput);
codex::gpio::Pin pin = codex::gpio::pin_11;
codex::gpio::setPinAsOutput(pin);
while (strength >= 0)
{
codex::time::TimeType interval = codex::time::milliseconds(1);
codex::time::delay(interval);
}
inputThread.join();

*/