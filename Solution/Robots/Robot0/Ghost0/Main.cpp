#include <iostream>
#include <SpehsEngine/SpehsEngine.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/BatchManager.h>
#include <SpehsEngine/AudioEngine.h>
#include <SpehsEngine/Camera2D.h>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/Time.h>
#include <Codex/IOService.h>
#include <Codex/SocketTCP.h>
#include <Codex/Codex.h>
#include <Codex/Log.h>
#include <string>

bool receiveHandler(codex::protocol::ReadBuffer& buffer)
{
	//Print out buffer contents...
	std::string str;
	buffer.read(str);
	codex::log::info("Received packet (str): " + str);
	return true;
}
int ghost(codex::SocketTCP& socket)
{
	codex::log::info("Running Ghost0...");

	//Socket test
	codex::protocol::WriteBuffer buffer;
	buffer.write(std::string("YO GET SPOOKD BY THE GHOST"));
	socket.startReceiving(std::bind(receiveHandler, std::placeholders::_1));
	socket.sendPacket(buffer);

	while (1)
	{
		if (!socket.isConnected())
		{
			codex::log::info("Shell disconnected.");
			break;
		}
	}
	return 0;
}

int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);
	codex::log::info("Ghost0 initializing...");

	//{
	//	codex::IOService ioservice;
	//	codex::SocketTCP socket0(ioservice);
	//	socket0.connect(argv[1], std::atoi(argv[2]));
	//}
	//Ghost socket
	codex::IOService ioservice;
	codex::SocketTCP socket(ioservice);
	socket.resizeReceiveBuffer(64000);

	if (argc == 3)
	{//Connect

		//Remote endpoint
		const std::string address = argv[1];
		const int port = std::atoi(argv[2]);

		//Some endpoint validation...
		int periodCount = 0;
		bool invalidCharacters = false;
		for (size_t i = 0; i < address.size(); i++)
		{
			if (address[i] == '.')
				periodCount++;
			else if (address[i] < 48 || address[i] > 57)
				invalidCharacters = true;
		}
		if (periodCount != 3 || invalidCharacters || address.size() > 15)
		{
			codex::log::error("Provided server address is invalid: " + address);
			return 1;
		}
		if (port < 0 || port > std::numeric_limits<uint16_t>::max())
		{
			codex::log::error("Provided server port is invalid: " + std::string(argv[2]));
			return 1;
		}

		//Try to connect
		codex::log::info("Connecting to the shell at " + address + ", port: " + std::to_string(port));
		if (socket.connect(address.c_str(), port))
		{//Connected
			codex::log::info("Successfully connected to the shell!");
			const int result = ghost(socket);
			socket.disconnect();
			return result;
		}
		else
		{//Failed to connect
			codex::log::error("Failed to connect to the provided shell endpoint! Press enter to exit the ghost...");
			return 1;
		}
	}
	else
	{//Incorrect command line argument usage
		codex::log::error("Incorrect usage of command line arguments! Correct usage: [1]shell endpoint's address [2]shell endpoint's port");
		return 1;
	}
	
	/*
	codex::log::info("Ghost0 initializing...");

	codex::log::info(std::to_string(argc));
	for (int i = 0; i < argc; i++)
		std::cout << "\n" << argv[i];

	spehs::initialize("Aria");
	spehs::Camera2D camera;
	spehs::BatchManager batchManager(&camera, "Aria");
	bool running = true;
	while (running)
	{
		spehs::time::update();
		inputManager->update();
		spehs::audio::AudioEngine::update();
		spehs::console::update();
		if (inputManager->isQuitRequested() || inputManager->isKeyPressed(KEYBOARD_ESCAPE))
			running = false;
		spehs::getMainWindow()->renderBegin();
		batchManager.render();
		spehs::console::render();
		spehs::getMainWindow()->renderEnd();
	}
	spehs::uninitialize();
	codex::log::info("Ghost0 stopped.");
	std::getchar();
	*/
	codex::uninitialize();
}