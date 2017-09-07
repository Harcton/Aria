#include "Shell.h"
#include <functional>

namespace codex
{

	Shell::Shell()
		: ioService()
		, socket(ioService)
		, stopRequested(false)
	{
		socket.resizeReceiveBuffer(64000);
	}
	
	Shell::~Shell()
	{

	}

	int Shell::start(const int argc, const char** argv)
	{
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
			socket.startAccepting(port, std::bind(&Shell::onAccept, this, std::placeholders::_1));

			while (!socket.isConnected() && socket.isAccepting())
			{ /*Block until accepting has come to a resolution*/ }

			if (socket.isConnected())
			{
				run();
				return 0;
			}
			return 1;
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

			//try to connect
			codex::log::info("connecting to the server at " + address + ", port: " + std::to_string(port));
			if (socket.connect(address.c_str(), port))
			{//connected
				codex::log::info("Successfully connected to the server!");

				//codex::log::info("Requesting ghost...");
				//if (socket.requestGhost("Ghost0"))
				//{
				//	codex::log::info("Ghost retrieved! Starting the shell program...");
				//	const int result = shell(socket);
				//	socket.disconnect();
				//	return result;
				//}
				//else
				//{
				//	codex::log::error("Could not retrieve ghost!");
				//	return 1;
				//}

				run();
				return 0;
			}
			else
			{//Failed to connect
				codex::log::error("Failed to connect to the provided server endpoint! Press enter to exit the shell...");
				return 1;
			}
		}
		else
		{//Incorrect command line argument usage
			codex::log::error("Incorrect usage of command line arguments! Correct usage: [1]ghost endpoint's address [2]ghost endpoint's port");
			return 1;
		}
	}

	void Shell::stop()
	{
		stopRequested = true;
	}

	void Shell::run()
	{
		//Start receiving
		const bool quitOnSocketDisconnect = socket.isConnected();
		if (socket.isConnected())
			socket.startReceiving(std::bind(&Shell::internalReceiveHandler, this, std::placeholders::_1));

		//Shell loop
		onStart();
		while (!stopRequested)
		{
			update();
			if (quitOnSocketDisconnect && !socket.isConnected())
				stop();
		}
		onStop();

		//Stop receiving
		if (socket.isReceiving())
			socket.stopReceiving();
	}

	bool Shell::onAccept(SocketTCP& socket)
	{
		if (socket.isConnected())
		{
			log::info("Remote connection accepted.");
		}
		else
		{
			log::info("Failed to accept remote connection.");
		}
		return false;//Do not start accepting again
	}

	bool Shell::internalReceiveHandler(codex::protocol::ReadBuffer& buffer)
	{
		receiveHandler(buffer);
		return true;
	}

	bool Shell::sendPacket(const protocol::WriteBuffer& buffer)
	{
		return socket.sendPacket(buffer) == buffer.getOffset();
	}
}