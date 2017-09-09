#include "Actor.h"

namespace codex
{
	Actor::Actor()
		: ioService()
		, socket(ioService)
		, stopRequested(false)
	{
		socket.resizeReceiveBuffer(64000);
	}

	Actor::~Actor()
	{

	}

	int Actor::start(const int argc, const char** argv)
	{
#ifdef GHOST_CODEX
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
			if (connect(address.c_str(), port))
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

#elif SHELL_CODEX
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
			startAccepting(port);

			const time::TimeType second = time::seconds(1);
			int elapsedSeconds = 0;
			time::TimeType beginTime = time::getRunTime();
			while (!isConnected() && isAccepting())
			{
				/*Block until accepting has come to a resolution*/
				if (time::getRunTime() - beginTime >= second)
				{
					beginTime = time::getRunTime();
					log::info("Elapsed seconds: " + std::to_string(++elapsedSeconds));
				}
			}

			if (isConnected())
			{
				run();
				return 0;
			}
			return 1;
		}
		else if (argc == 3)
		{//Connect

			//Remote endpoint
			const protocol::AddressType address = argv[1];
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
			if (connect(address, port))
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
			log::error("Incorrect usage of command line arguments! Correct usage: [1]ghost endpoint's address [2]ghost endpoint's port");
			return 1;
		}
#endif
	}

	void Actor::run()
	{
		//Start receiving
		if (!socket.isConnected())
			return;
		startReceiving();
		
		//Shell loop
		onStart();
		time::TimeType deltaTime = 0;
		time::TimeType beginUpdateTime = 0;
		time::TimeType beginPacketsTime = 0;
		const time::TimeType warningThresholdTime = time::seconds(0.5f);
		do
		{
			beginUpdateTime = time::getRunTime();
			update(deltaTime);
			if (time::getRunTime() - beginUpdateTime >= warningThresholdTime)
				log::warning("Actor update took " + std::to_string(float(time::getRunTime() - beginUpdateTime) / codex::time::conversionRate::millisecond) + " ms!");

			beginPacketsTime = time::getRunTime();
			{//Handle received packets. Enable thread lock for socket for the duration of this process.
				socket.enableThreadLock();
				for (size_t i = 0; i < receivedPackets.size(); i++)
				{
					protocol::ReadBuffer readBuffer(receivedPackets[i].data(), receivedPackets[i].size(), getReadEndianness());
					receiveHandler(readBuffer);
				}
				receivedPackets.clear();
				socket.releaseThreadLock();
			}
			if (time::getRunTime() - beginPacketsTime >= warningThresholdTime)
				log::warning("Packet receive handling took " + std::to_string(float(time::getRunTime() - beginPacketsTime) / codex::time::conversionRate::millisecond) + " ms!");

			if (!socket.isConnected())
				stop();

			deltaTime = time::getRunTime() - beginUpdateTime;
		} while (!stopRequested);
		onStop();
				
		//Stop receiving
		if (socket.isReceiving())
			socket.stopReceiving();		
	}

	void Actor::stop()
	{
		stopRequested = true;
	}

	void Actor::startReceiving()
	{
		socket.startReceiving(std::bind(&Actor::internalReceiveHandler, this, std::placeholders::_1));
	}

	void Actor::stopReceiving()
	{
		socket.stopReceiving();
	}

	void Actor::startAccepting(const protocol::PortType& port)
	{
		socket.startAccepting(port, std::bind(&Actor::onAccept, this, std::placeholders::_1));
	}

	void Actor::stopAccepting()
	{
		socket.stopAccepting();
	}

	bool Actor::connect(const protocol::AddressType& address, const protocol::PortType& port)
	{
		return socket.connect(address, port);
	}

	bool Actor::internalReceiveHandler(codex::protocol::ReadBuffer& buffer)
	{
		//Push to received packets. Call receive handler between updates.
		receivedPackets.push_back(std::vector<unsigned char>());
		receivedPackets.back().resize(buffer.getBytesRemaining());
		memcpy(receivedPackets.back().data(), buffer[0], buffer.getBytesRemaining());
		return true;
	}

	bool Actor::onAccept(SocketTCP& socket)
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

	bool Actor::sendPacket(const protocol::WriteBuffer& buffer)
	{
		return socket.sendPacket(buffer) == buffer.getOffset();
	}

	bool Actor::isConnected() const
	{
		return socket.isConnected();
	}

	bool Actor::isReceiving() const
	{
		return socket.isReceiving();
	}

	bool Actor::isAccepting() const
	{
		return socket.isAccepting();
	}

	protocol::Endianness Actor::getReadEndianness() const
	{
#ifdef GHOST_CODEX
		return socket.getRemoteEndianness();
#elif SHELL_CODEX
		return protocol::Endianness::local;
#endif
	}

	protocol::Endianness Actor::getWriteEndianness() const
	{
#ifdef GHOST_CODEX
		return socket.getRemoteEndianness();
#elif SHELL_CODEX
		return protocol::Endianness::local;
#endif
	}
}