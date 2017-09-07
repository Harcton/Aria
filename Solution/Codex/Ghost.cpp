#ifdef GHOST_CODEX
#include "Ghost.h"


namespace codex
{
	Ghost::Ghost()
		: ioService()
		, socket(ioService)
		, stopRequested(false)
	{
		socket.resizeReceiveBuffer(64000);
	}


	Ghost::~Ghost()
	{
	}

	int Ghost::start(const int argc, const char** argv)
	{
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
				run();
				return 0;
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
	}

	void Ghost::run()
	{
		if (!socket.isConnected())
			return;

		//Start receiving
		socket.startReceiving(std::bind(&Ghost::internalReceiveHandler, this, std::placeholders::_1));

		//Shell loop
		onStart();
		while (!stopRequested)
		{
			update();
			if (!socket.isConnected())
				stop();
		}
		onStop();

		//Stop receiving
		if (socket.isReceiving())
			socket.stopReceiving();
	}

	void Ghost::stop()
	{
		stopRequested = true;
	}

	bool Ghost::internalReceiveHandler(codex::protocol::ReadBuffer& buffer)
	{
		receiveHandler(buffer);
		return true;
	}

	bool Ghost::sendPacket(const protocol::WriteBuffer& buffer)
	{
		return socket.sendPacket(buffer) == buffer.getOffset();
	}
}
#endif