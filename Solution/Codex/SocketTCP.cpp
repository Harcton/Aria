#include <assert.h>
#include <iostream>
#include <string>
#include "SocketTCP.h"
#include "Acceptor.h"
#include "IOService.h"
#include "Log.h"
#include "CodexTime.h"
#include "FileStream.h"
#include "RAIIVariableSetter.h"


namespace codex
{
	extern std::string workingDirectory;
	int debugLogLevel = 1;
	static const time::TimeType handshakeReceiveTimeout = time::seconds(10000);
	static const time::TimeType connectionTimeout = time::seconds(10000);

	SocketTCP::SocketTCP(IOService& _ioService)
		: ioService(_ioService)
		, socket(_ioService.getImplementationRef())
		, acceptor(nullptr)
		, codexAcceptThread(nullptr)
		, onReceiveCallback()
		, onAcceptCallback()
		, expectedBytes(0)
		, receiveBuffer(sizeof(size_t))
		, lastReceiveTime(0)
		, receiving(false)
		, accepting(false)
		, connected(false)
		, connecting(false)
		, handshakeSent(false)
		, handshakeReceived(false)
	{
#ifdef GHOST_CODEX
		remoteEndianness = codex::protocol::Endianness::undefined;
#endif
	}

	SocketTCP::~SocketTCP()
	{
		disconnect(protocol::DisconnectType::socketDestructor);
		stopAccepting();
		waitUntilFinishedReceiving();
		waitUntilFinishedAccepting();
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (acceptor)
		{
			delete acceptor;
			acceptor = nullptr;
		}
		if (codexAcceptThread)
		{
			delete codexAcceptThread;
			codexAcceptThread = nullptr;
		}
	}
	
	void SocketTCP::waitUntilFinishedReceiving()
	{
		bool wait = true;
		while (wait)
		{
			wait = isReceiving();
		}
	}

	void SocketTCP::waitUntilFinishedAccepting()
	{
		bool wait = true;
		while (wait)
		{
			wait = isAccepting();
		}

		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (codexAcceptThread)
		{
			codexAcceptThread->join();
			delete codexAcceptThread;
		}
	}

	void SocketTCP::waitUntilReceivedHandshake(const time::TimeType timeout)
	{
		bool wait;
		const time::TimeType beginTime = time::getRunTime();
		do
		{
			if (time::getRunTime() - beginTime > timeout)
				return;

			std::lock_guard<std::recursive_mutex> lock(mutex);
			wait = !handshakeReceived;
		} while (wait);
	}

	bool SocketTCP::connect(const protocol::Endpoint& endpoint)
	{
		return connect(endpoint.address, endpoint.port);
	}

	bool SocketTCP::connect(const protocol::AddressType& address_ipv4, const protocol::PortType& port)
	{
		RAIIMutexVariableSetter<bool, std::recursive_mutex> connectingSetter(connecting, true, mutex);

		{
			std::lock_guard<std::recursive_mutex> lock(mutex);

			if (connected)
			{
				log::info("SocketTCP::connect() failed to connect. Socket is already at the connected state!");
				return false;
			}

			//Start a new connection
			handshakeSent = false;
			handshakeReceived = false;

			boost::system::error_code error;
			boost::asio::ip::tcp::resolver resolverTCP(ioService.getImplementationRef());
			boost::asio::ip::tcp::resolver::query query(address_ipv4, std::to_string(port));
			boost::asio::ip::tcp::endpoint endpoint = *resolverTCP.resolve(query, error);
			if (error)
			{
				log::info("SocketTCP::connect() failed to resolve the endpoint. Boost asio error: " + error.message());
				return false;
			}

			try
			{
				socket.connect(endpoint, error);
			}
			catch (std::exception& exception)
			{
				log::info(std::string("SocketTCP::connect() failed to connect. Exception thrown: ") + exception.what());
				return false;
			}

			if (error)
			{
				log::info("SocketTCP::connect() failed to connect. Boost asio error: " + error.message());
				return false;
			}

			//Expect an incoming handshake after sending one
			startReceiving(onReceiveCallback);

			//Send the codex handshake (written in the local endianness)
			protocol::WriteBuffer buffer;
			protocol::Handshake handshake;
			handshake.write(buffer);
			if (handshakeSent = sendPacket(buffer, protocol::PacketType::handshake))
			{
				log::info("SocketTCP::connect() successfully sent handshake to the remote endpoint.");
			}
			else
			{
				log::info("SocketTCP::connect() failed to connect. Failed to send handshake.");
				return false;//If sending the handshake fails, connection was not successful
			}
		}

		//Wait until received handshake
		waitUntilReceivedHandshake(handshakeReceiveTimeout);
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			if (!handshakeReceived)
			{
				codex::log::info("SocketTCP::connect() failed to connect! No response handshake received!");
				return false;
			}
		}
		if (debugLogLevel >= 1)
			log::info("SocketTCP::connect() successfully received handshake from the remote endpoint. Socket is now in connected state.");
		
		//All done, socket is now at connected state!
		std::lock_guard<std::recursive_mutex> lock(mutex);
		connected = true;
		lastReceiveTime = time::getRunTime();

		return true;
	}

	void SocketTCP::disconnect()
	{
		disconnect(protocol::DisconnectType::unspecified);
	}

	void SocketTCP::disconnect(const protocol::DisconnectType disconnectType)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (!connected)
			return;

		if (disconnectType != protocol::DisconnectType::doNotSendDisconnectPacket)
		{//Try sending the disconnect packet before disconnecting
#ifdef GHOST_CODEX
			protocol::WriteBuffer buffer(remoteEndianness);
#else
			protocol::WriteBuffer buffer;
#endif
			buffer.write<protocol::DisconnectType>(disconnectType);
			sendPacket(buffer, protocol::PacketType::disconnect);
		}

		if (socket.is_open())
		{//Shut down the boost socket
			socket.shutdown(boost::asio::socket_base::shutdown_both);
			socket.close();
		}

		//Reset the connection state
		connected = false;
		handshakeSent = false;
		handshakeReceived = false;
#ifdef GHOST_CODEX
		remoteEndianness = protocol::Endianness::undefined;
#endif
	}

	void SocketTCP::stopReceiving()
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		socket.close();//TODO: this actually cancels all asynchronous operations, not just receiving...
	}

	void SocketTCP::stopAccepting()
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (acceptor)
			acceptor->close();
	}

	bool SocketTCP::sendPacket(const protocol::WriteBuffer& buffer, const protocol::PacketType packetType)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		boost::system::error_code error;

		if (!connected && !connecting && !accepting)
		{//Can only send user defined packets in the connected state
			log::info("SocketTCP: cannot send a packet. Socket is neither connected, connecting nor accepting.");
			return false;
		}

#ifdef GHOST_CODEX
		if (remoteEndianness != protocol::Endianness::undefined && buffer.getEndianness() != remoteEndianness)
		{
			log::info("SocketTCP: cannot send a packet! Write buffer was written in a wrong byte order!");
			return false;
		}
#endif

		//Codex header
		const ExpectedBytesType dataBufferSize = buffer.getOffset();
		const ExpectedBytesType headerBytesValue = buffer.getOffset() + sizeof(packetType);
#ifdef GHOST_CODEX
		protocol::WriteBuffer headerBuffer(connected ? remoteEndianness : protocol::Endianness::local);
#else
		protocol::WriteBuffer headerBuffer;
#endif
		headerBuffer.write(headerBytesValue);
		headerBuffer.write(packetType);
		const size_t headerBufferSize = headerBuffer.getOffset();
		size_t offset = 0;
		while (offset < headerBufferSize)
		{//Keep sending data until the whole header has been sent
			offset += socket.write_some(boost::asio::buffer(headerBuffer[offset], headerBufferSize - offset), error);
			if (error)
			{//Error occured while sending data...
				codex::log::warning("SocketTCP: failed to send packet's codex header! Boost asio error: " + error.message());
				return false;
			}
		}

		//Data
		offset = 0;
		while (offset < dataBufferSize)
		{//Keep sending data until all data has been sent
			offset += socket.write_some(boost::asio::buffer(buffer[offset], dataBufferSize - offset), error);
			if (error)
			{//Error occured while sending data...
				codex::log::warning("SocketTCP: failed to send packet! Boost asio error: " + error.message());
				return false;
			}
		}

		if (debugLogLevel >= 2)
			log::info("SocketTCP: packet sent. Contents: 4(packet byte size) + 1(packet type) + " + std::to_string(buffer.getOffset()) + "(data)");
		return true;
	}

	bool SocketTCP::resizeReceiveBuffer(const size_t newSize)
	{
		if (newSize < sizeof(ExpectedBytesType))
			return false;
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (receiving)
			return false;
		receiveBuffer.resize(newSize);
		return true;
	}

	bool SocketTCP::startReceiving(const std::function<bool(protocol::ReadBuffer&)> callbackFunction)
	{
		if (isReceiving())
		{
			log::info("SocketTCP failed to start receiving. Socket is already receiving.");
			return false;
		}

		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (!connected && !connecting && !accepting)
		{
			codex::log::warning("SocketTCP: failed to start receiving. Socket is neither connected, connecting nor accepting.");
			return false;
		}
		if (!socket.is_open())
		{//This should never happen as the connected status should guarantee socket being open...
			log::info("SocketTCP failed to start receiving. Socket has not been opened.");
			return false;
		}

		receiving = true;
		lastReceiveTime = time::getRunTime();
		onReceiveCallback = callbackFunction;
		if (expectedBytes == 0)
		{//Receive header
			boost::asio::async_read(socket, boost::asio::buffer(&receiveBuffer[0], sizeof(expectedBytes)),
				boost::bind(&SocketTCP::receiveHandler,
					this, boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
		else
		{//Receive data
			boost::asio::async_read(socket, boost::asio::buffer(&receiveBuffer[0], expectedBytes),
				boost::bind(&SocketTCP::receiveHandler,
					this, boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}

		if (debugLogLevel >= 2)
		{
			if (expectedBytes > 0)
				log::info("SocketTCP successfully started receiving. Expecting bytes: " + std::to_string(expectedBytes) + ".");
			else
				log::info("SocketTCP successfully started receiving. Expecting bytes header.");
		}
		return true;
	}

	void SocketTCP::receiveHandler(const boost::system::error_code& error, std::size_t bytes)
	{
		if (debugLogLevel >= 2)
			log::info("SocketTCP receive handler received " + std::to_string(bytes) + " bytes.");

		std::lock_guard<std::recursive_mutex> lock(mutex);
		receiving = false;
		lastReceiveTime = time::getRunTime();

		if (error)
		{
			if (error == boost::asio::error::eof)
			{//Connection gracefully closed
				log::info("SocketTCP disconnected. Remote socket closed connection.");
				disconnect(protocol::DisconnectType::doNotSendDisconnectPacket);
				return;
			}
			else if (error == boost::asio::error::connection_reset)
			{//Disconnect
				log::info("SocketTCP disconnected. Remote socket closed connection.");
				disconnect(protocol::DisconnectType::doNotSendDisconnectPacket);
				return;
			}
			else if (error == boost::asio::error::connection_aborted ||
				error == boost::asio::error::connection_refused ||
				error == boost::asio::error::bad_descriptor ||
				error == boost::asio::error::operation_aborted)
			{
				log::info("Boost asio error: " + std::to_string(error.value()));
				if (error == boost::asio::error::connection_aborted)
					log::info("Closing client: boost asio error: connection_aborted");
				if (error == boost::asio::error::connection_refused)
					log::info("Closing client: boost asio error: connection_refused");
				if (error == boost::asio::error::bad_descriptor)
					log::info("Closing client: boost asio error: bad_descriptor");
				if (error == boost::asio::error::operation_aborted)
					log::info("Closing client: boost asio error: operation_aborted");
				disconnect(protocol::DisconnectType::doNotSendDisconnectPacket);
				return;
			}
			else
			{//Ignored error
				log::info("Ignored ASIO error: " + std::to_string(error.value()) + ": " + error.message());
			}
		}

		if (bytes)
		{
			if (expectedBytes == 0)
			{//Header received
				memcpy(&expectedBytes, &receiveBuffer[0], sizeof(expectedBytes));
				if (!connected)
				{
					/*
						Socket is not in connected state, must deduct remote endianness from expected bytes
						We can make a wild assumption that the least significant bits form a binary number larger than the most significant bits.
						This is because the first packet of each connection should always be just a small handshake packet.
					*/
					const uint32_t leastSignificantBits = 0x0000FFFF & expectedBytes;
					const uint32_t mostSignificantBits = (0xFFFF0000 & expectedBytes) >> 16;
					if (mostSignificantBits > leastSignificantBits)
					{//Inverse expected bytes byte ordering
						const uint32_t reverted = expectedBytes;
						for (size_t i = 0; i < 4; i++)
							memcpy(&(((unsigned char*)&expectedBytes)[i]), &(((unsigned char*)&reverted)[3 - i]), 1);
					}
#ifdef GHOST_CODEX
					if (mostSignificantBits > leastSignificantBits)
						remoteEndianness = protocol::Endianness::inverted;
					else
						remoteEndianness = protocol::Endianness::local;
#endif
				}
				startReceiving(onReceiveCallback);
			}
			else if (expectedBytes == bytes)
			{//Data received

				//Read buffer
#ifdef SHELL_CODEX
				protocol::ReadBuffer buffer(&receiveBuffer[0], bytes);
#else
				protocol::ReadBuffer buffer(&receiveBuffer[0], bytes, remoteEndianness);
#endif
				const bool keepReceiving = codexReceiveHandler(buffer);
				expectedBytes = 0;//Begin to expect header next
				if (keepReceiving)
					startReceiving(onReceiveCallback);
			}
			else
			{
				log::warning("SocketTCP received unexpected amount of bytes! Canceling receiving!");
			}
		}
	}

	bool SocketTCP::codexReceiveHandler(codex::protocol::ReadBuffer& buffer)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);

		//Read packet type
		protocol::PacketType packetType;
		buffer.read(packetType);
		if (debugLogLevel >= 2)
			log::info("SocketTCP codex receive handler received packet of type: " + std::to_string((int)packetType) + ", " + std::to_string(buffer.getBytesRemaining()) + " bytes.");

		//Process packet
		switch (packetType)
		{
		case protocol::PacketType::undefined:
		{
			//Create new read buffer that hides the codex layer
#ifdef GHOST_CODEX
			codex::protocol::ReadBuffer dataBuffer(buffer[buffer.getOffset()], buffer.getBytesRemaining(), remoteEndianness);
#else
			codex::protocol::ReadBuffer dataBuffer(buffer[buffer.getOffset()], buffer.getBytesRemaining());
#endif
			if (debugLogLevel >= 2)
				codex::log::info("SocketTCP received user defined packet. Bytes: " + std::to_string(dataBuffer.getBytesRemaining()));
			if (onReceiveCallback)
				return onReceiveCallback(dataBuffer);
			else
				return false;
		}
		case protocol::PacketType::disconnect:
		{
			protocol::DisconnectType disconnectType;
			buffer.read<protocol::DisconnectType>(disconnectType);
			disconnect(protocol::DisconnectType::doNotSendDisconnectPacket);
			return false;
		}
		case protocol::PacketType::handshake:
		{
			if (handshakeReceived)
			{
				log::warning("A handshake for the current connection has already been received!");
				return false;
			}
			protocol::Handshake handshake;
			handshake.read(buffer);

			if (handshake.isValid())
			{//VALID HANDSHAKE
				handshakeReceived = true;
#ifdef GHOST_CODEX //Set byte endianness
				remoteEndianness = handshake.getEndianness();
#endif
				if (debugLogLevel >= 1)
					log::info("SocketTCP valid handshake received.");
			}
			else
			{//INVALID HANDSHAKE -> DISCARD
				handshakeReceived = true;
				log::warning("Received an invalid codex handshake!");
			}
		}
		}
		return false;//Do not start receiving again!
	}

	bool SocketTCP::startAccepting(const protocol::PortType port, const std::function<void(SocketTCP&)> callbackFunction)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (accepting)
		{
			log::info("SocketTCP failed to start accepting! Socket is already accepting!");
			return false;
		}
		if (connected)
		{
			codex::log::warning("SocketTCP failed start accepting! Socket is currently connected!");
			return false;
		}
		assert(!handshakeSent);
		assert(!handshakeReceived);

		if (acceptor == nullptr)
			acceptor = new boost::asio::ip::tcp::acceptor(ioService.getImplementationRef());
		boost::system::error_code error;
		const boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);

		//Open acceptor
		acceptor->open(endpoint.protocol(), error);
		if (error)
		{
			log::error("Failed to open acceptor! Boost asio error: " + error.message());
			return false;
		}

		//Bind acceptor
		acceptor->bind(endpoint, error);
		if (error)
		{
			log::error("Failed to bind acceptor! Boost asio error: " + error.message());
			return false;
		}
		if (!acceptor->is_open())
		{
			log::error("Boost acceptor failed to open!");
			return false;
		}

		//Make acceptor listen
		acceptor->listen(boost::asio::socket_base::max_connections, error);
		if (error)
		{
			log::error("Failed to make acceptor listen! Boost asio error: " + error.message());
			return false;
		}

		//Set the callback function
		onAcceptCallback = callbackFunction;

		//Start accepting asynchoronously
		accepting = true;
		acceptor->async_accept(socket, boost::bind(&SocketTCP::onAccept, this, boost::asio::placeholders::error));

		return true;
	}

	void SocketTCP::onAccept(const boost::system::error_code error)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		assert(!isConnected());
		assert(!isReceiving());
		assert(isAccepting());
		assert(acceptor);
		assert(!codexAcceptThread);
		acceptor->close();
		if (error)
		{
			log::warning("SocketTCP failed to accept an incoming connection! Boost asio error: " + error.message() + "Accepting has stopped.");
			disconnect(protocol::DisconnectType::doNotSendDisconnectPacket);
			accepting = false;
			return;
		}
		codexAcceptThread = new std::thread(&SocketTCP::codexAccept, this);
	}

	void SocketTCP::codexAccept()
	{
		//Wait until received remote handshake
		if (debugLogLevel >= 1)
			codex::log::info("Accepting SocketTCP expecting a handshake...");

		//Start expecting an incoming handshake (connector sends first)
		startReceiving(onReceiveCallback);
		waitUntilReceivedHandshake(handshakeReceiveTimeout);

		{//Check if received the handshake in time
			std::lock_guard<std::recursive_mutex> lock(mutex);
			if (!handshakeReceived)
			{
				codex::log::info("SocketTCP failed to accept an incoming connection! No response handshake received!");
				if (onAcceptCallback)
					onAcceptCallback(*this);
				disconnect(protocol::DisconnectType::doNotSendDisconnectPacket);
				return;
			}
		}
		if (debugLogLevel >= 1)
			codex::log::info("Accepting SocketTCP received a handshake.");
		
		//Send a response handshake
		protocol::WriteBuffer buffer;
		protocol::Handshake handshake;
		handshake.write(buffer);
		if (sendPacket(buffer, protocol::PacketType::handshake))
		{
			handshakeSent = true;
			if (debugLogLevel >= 1)
				codex::log::info("Accepting SocketTCP sent a handshake.");
		}
		else
		{
			{
				std::lock_guard<std::recursive_mutex> lock(mutex);
				accepting = false;
			}
			codex::log::info("SocketTCP failed to accept an incoming connection! Could not send handshake!");
			if (onAcceptCallback)
				onAcceptCallback(*this);
			disconnect(protocol::DisconnectType::doNotSendDisconnectPacket);
			return;
		}

		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			accepting = false;
			connected = true;
			lastReceiveTime = time::getRunTime();
		}

		//Socket is now in the connected status! Make the onAcceptCallback callback.
		if (onAcceptCallback)
			onAcceptCallback(*this);
	}

	std::string SocketTCP::getRemoteAddress() const
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (!isConnected())
		{
			log::info("SocketTCP: cannot retrieve remote endpoint address! Socket is not connected!");
			return "0.0.0.0";
		}
		boost::asio::ip::tcp::endpoint endpoint = socket.remote_endpoint();
		boost::asio::ip::address address = endpoint.address();
		boost::asio::ip::address_v4 address_v4 = address.to_v4();
		return address_v4.to_string();
	}

	protocol::PortType SocketTCP::getRemotePort() const
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (!isConnected())
		{
			log::info("SocketTCP: cannot retrieve remote endpoint port! Socket is not connected!");
			return 0;
		}
		boost::asio::ip::tcp::endpoint endpoint = socket.remote_endpoint();
		return endpoint.port();
	}

	protocol::Endpoint SocketTCP::getRemoteEndpoint() const
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (!isConnected())
		{
			log::info("SocketTCP: cannot retrieve remote endpoint port! Socket is not connected!");
			return protocol::Endpoint("0.0.0.0", 0);
		}
		return protocol::Endpoint(socket.remote_endpoint().address().to_v4().to_string(), socket.remote_endpoint().port());
	}
	
	bool SocketTCP::isAccepting() const
	{
		std::lock_guard<std::recursive_mutex> locks(mutex);
		return accepting;
	}
	
	bool SocketTCP::isReceiving() const
	{
		std::lock_guard<std::recursive_mutex> locks(mutex);
		return receiving;
	}

	bool SocketTCP::isConnected() const
	{
		std::lock_guard<std::recursive_mutex> locks(mutex);
		if (time::getRunTime() - lastReceiveTime >= connectionTimeout)
			return false;
		return connected;
	}

#ifdef GHOST_CODEX
	protocol::Endianness SocketTCP::getRemoteEndianness() const
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (!isConnected())
		{
			log::info("SocketTCP: cannot retrieve remote endpoint byte endianness! Socket is not connected!");
			return protocol::Endianness::undefined;
		}
		return remoteEndianness;
	}
#endif

	static const uint64_t magicYes = 0xBAABAABBBBADAB00;

	ShellSocketTCP::ShellSocketTCP(IOService& io)
		: SocketTCP(io)
		, requestGhostResponseReceived(false)
	{

	}

	ShellSocketTCP::~ShellSocketTCP()
	{

	}

	bool ShellSocketTCP::internalReceiveHandler(codex::protocol::ReadBuffer& buffer)
	{
		requestGhostResponseReceived = true;
		buffer.read(requestGhostReceivedResponse);
		if (requestGhostReceivedResponse == magicYes)
		{//Read the rest
			buffer.read(requestGhostReceivedResponseAddress);
			buffer.read(requestGhostReceivedResponsePort);
		}
		return false;
	}

	bool ShellSocketTCP::requestGhost(const std::string& ghostName)
	{
		//TODO: do not limit this function for just one thread...
		if (!requestGhostMutex.try_lock())
		{
			log::warning("requestGhost() failed: another thread is already runnig this process!");
			return false;
		}
		requestGhostMutex.unlock();
		std::lock_guard<std::mutex> lock(requestGhostMutex);

		if (!isConnected())
		{
			log::warning("requestGhost() failed: passed socket is not connected to an endpoint! Connect first, then request the ghost!");
			return false;
		}
		if (isReceiving())
		{
			log::warning("requestGhost() failed: passed socket is already receiving!");
			return false;
		}

		codex::protocol::WriteBuffer buffer;
		buffer.write(ghostName);
		if (!sendPacket(buffer))
		{
			log::info("requestGhost() failed: sending a packet failed!");
			return false;
		}

		requestGhostReceivedResponse = false;
		startReceiving(std::bind(&ShellSocketTCP::internalReceiveHandler, this, std::placeholders::_1));
		codex::time::TimeType startTime = time::getRunTime();
		while (!requestGhostReceivedResponse)
		{
			codex::time::delay(codex::time::milliseconds(1));
			if (time::getRunTime() - startTime > codex::time::seconds(5))
			{
				codex::log::info("requestGhost() failed: the remote socket did not respond within the given time window.");
				stopReceiving();
				return false;
			}
		}

		if (requestGhostReceivedResponse == magicYes)
		{//Ghost has been deployed to the specified endpoint
			const protocol::AddressType remoteAddress = getRemoteAddress();
			const protocol::PortType remotePort = getRemotePort();
			disconnect(protocol::DisconnectType::switchEndpoint);

			//Connect to ghost
			if (!connect(requestGhostReceivedResponseAddress.c_str(), requestGhostReceivedResponsePort))
			{
				codex::log::info("requestGhost() failed: could not connect to the provided ghost.");
				//Try to reconnect to the previously connected endpoint
				if (!connect(remoteAddress.c_str(), remotePort))
					codex::log::info("requestGhost() failed to reconnect back to the previous endpoint!");

				return false;
			}

		}
		else
		{
			codex::log::info("requestGhost() failed: the remote socket responded, but it could not currently provide a ghost with the specified name.");
			return true;
		}
	}


#ifdef GHOST_CODEX
	GhostSocketTCP::GhostSocketTCP(IOService& io)
		: SocketTCP(io)
	{

	}

	GhostSocketTCP::~GhostSocketTCP()
	{

	}

	bool GhostSocketTCP::ghostRequestHandler(codex::protocol::ReadBuffer& buffer)
	{
		std::string ghostName;
		buffer.read(ghostName);
		const std::string ghostPath = workingDirectory + "/" + ghostName + ".exe";
		if (fileExists(ghostPath))
		{//Ghost file exists
			//Search for the specified ghost program on local drive. If ghost program exists, launch it and pass it the remote endpoint to connect to.

#ifdef _WIN32
			// additional information
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			// set the size of the structures
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));
			// start the program up
			CreateProcess(ghostPath.c_str(), // format: "E:/Ohjelmointi/Projects/Aria/Solution/bin/Ghost0.exe"
				"param0",		// Command line
				NULL,           // Process handle not inheritable
				NULL,           // Thread handle not inheritable
				FALSE,          // Set handle inheritance to FALSE
				CREATE_NEW_CONSOLE,//Creation flags
				NULL,           // Use parent's environment block
				NULL,           // Use parent's starting directory 
				&si,            // Pointer to STARTUPINFO structure
				&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
			);
			// Close process and thread handles.
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
#elif
#warning Missing implementation for ghost launching on current platform
#endif
		}
		else
		{//Ghost file doesn't exist

		}
		return false;//=do not start receiving again
	}
#endif
}