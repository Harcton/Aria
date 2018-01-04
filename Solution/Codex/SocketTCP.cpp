#include <assert.h>
#include <iostream>
#include <string>
#include "SocketTCP.h"
#include "Acceptor.h"
#include "IOService.h"
#include "RAIIVariableSetter.h"
#include "SpehsEngine/Core/Log.h"
#include "SpehsEngine/Core/Time.h"
#include "SpehsEngine/Core/StringOperations.h"


namespace codex
{
	extern std::string workingDirectory;
	/*
		level 1: prints most essential state changes.
		level 2: prints some network traffic numbers.
		level 3: prints receive buffer in hex string.
	*/
	int debugLogLevel = 0;
	static const spehs::time::Time handshakeReceiveTimeout = spehs::time::fromSeconds(10000);
	static const spehs::time::Time connectionTimeout = spehs::time::fromSeconds(10000);

	SocketTCP::Id SocketTCP::nextId = 0;
	SocketTCP::SocketTCP(IOService& _ioService)
		: id(nextId++)
		, ioService(_ioService)
		, socket(_ioService.getImplementationRef())
		, acceptor(nullptr)
		, codexAcceptThread(nullptr)
		, onReceiveCallback()
		, onAcceptCallback()
		, expectedBytes(0)
		, receiveBuffer(512)
		, lastReceiveTime(0)
		, receiving(false)
		, accepting(false)
		, connected(false)
		, connecting(false)
		, handshakeSent(false)
		, handshakeReceived(false)
		, onAcceptCallbackQueued(false)
	{

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
		SPEHS_ASSERT(codexAcceptThread == nullptr);

		clearReceivedPackets();
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
			codexAcceptThread = nullptr;
		}
	}

	void SocketTCP::waitUntilReceivedHandshake(const spehs::time::Time timeout)
	{
		bool wait;
		const spehs::time::Time beginTime = spehs::time::now();
		do
		{
			if (spehs::time::now() - beginTime > timeout)
				return;

			std::lock_guard<std::recursive_mutex> lock(mutex);
			wait = !handshakeReceived;
		} while (wait);
	}

	void SocketTCP::clearReceivedPackets()
	{
		std::lock_guard<std::recursive_mutex> lock2(receivedPacketsMutex);
		for (size_t i = 0; i < receivedPackets.size(); i++)
			delete receivedPackets[i];
		receivedPackets.clear();
	}

	bool SocketTCP::connect(const protocol::Endpoint& endpoint)
	{
		RAIIMutexVariableSetter<bool, std::recursive_mutex> connectingSetter(connecting, true, mutex);

		//Stop receiving (outside the main mutex!)
		stopReceiving();

		{
			std::lock_guard<std::recursive_mutex> lock(mutex);

			if (connected)
			{
				spehs::log::info("SocketTCP::connect: failed to connect. Socket is already at the connected state!");
				return false;
			}

			//Start a new connection
			handshakeSent = false;
			handshakeReceived = false;

			//Resolve the remote endpoint
			boost::system::error_code error;
			boost::asio::ip::tcp::resolver resolverTCP(ioService.getImplementationRef());
			boost::asio::ip::tcp::resolver::query query(endpoint.address, std::to_string(endpoint.port));
			boost::asio::ip::tcp::endpoint asioEndpoint = *resolverTCP.resolve(query, error);
			if (error)
			{
				spehs::log::info("SocketTCP::connect: failed to resolve the endpoint. Boost asio error: " + error.message());
				return false;
			}
			else
			{
				if (debugLogLevel >= 1)
					spehs::log::info("SocketTCP::connect: successfully resolved the remote endpoint.");
			}

			//Connect to the remote boost socket
			try
			{
				socket.connect(asioEndpoint, error);
			}
			catch (std::exception& exception)
			{
				spehs::log::info(std::string("SocketTCP::connect: failed to connect. Exception thrown: ") + exception.what());
				return false;
			}
			if (error)
			{
				spehs::log::info("SocketTCP::connect: failed to connect. Boost asio error: " + error.message());
				return false;
			}
			else
			{
				if (debugLogLevel >= 1)
					spehs::log::info("SocketTCP::connect: successfully connected the remote boost::asio::ip::tcp::socket.");
			}

			//Expect an incoming handshake after sending one
			startReceiving(onReceiveCallback);

			//Send the codex handshake
			protocol::WriteBuffer buffer;
			const protocol::Handshake handshake;
			buffer.write(handshake);
			handshakeSent = sendPacket(buffer, protocol::PacketType::handshake);
			if (handshakeSent)
			{
				if (debugLogLevel >= 1)
					spehs::log::info("SocketTCP::connect: successfully sent handshake to the remote endpoint.");
			}
			else
			{
				spehs::log::info("SocketTCP::connect: failed to connect. Failed to send handshake.");
				return false;//If sending the handshake fails, connection was not successful
			}
		}

		//Wait until received handshake
		waitUntilReceivedHandshake(handshakeReceiveTimeout);
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			if (!handshakeReceived)
			{
				spehs::log::info("SocketTCP::connect: failed to connect! No response handshake received!");
				return false;
			}
		}
		if (debugLogLevel >= 1)
			spehs::log::info("SocketTCP::connect: successfully received handshake from the remote endpoint. Socket is now in connected state.");
		
		//All done, socket is now at connected state!
		std::lock_guard<std::recursive_mutex> lock(mutex);
		connected = true;
		lastReceiveTime = spehs::time::now();

		return true;
	}

	void SocketTCP::disconnect()
	{
		disconnect(protocol::DisconnectType::unspecified);
	}

	void SocketTCP::disconnect(const protocol::DisconnectType disconnectType)
	{
		//Stop receiving (outside the main mutex!)
		stopReceiving();

		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (!connected)
			return;

		if (disconnectType != protocol::DisconnectType::doNotSendDisconnectPacket)
		{//Try sending the disconnect packet before disconnecting
			protocol::WriteBuffer buffer;
			buffer.write(disconnectType);
			sendPacket(buffer, protocol::PacketType::disconnect);
		}
		
		//Reset the connection state
		connected = false;
		handshakeSent = false;
		handshakeReceived = false;
	}

	void SocketTCP::stopReceiving()
	{
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			if (socket.is_open())
			{
				try
				{
					socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
					socket.close();//TODO: this actually cancels all asynchronous operations, not just receiving...
				}
				catch (std::exception& e)
				{
					spehs::log::info(e.what());
				}
			}
		}
		while (isReceiving())
		{
			//Blocks
		}
	}

	void SocketTCP::stopAccepting()
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (acceptor)
		{
			try
			{
				acceptor->close();
			}
			catch (std::exception& e)
			{
				spehs::log::info(e.what());
			}
		}
	}

	bool SocketTCP::sendPacket(const protocol::WriteBuffer& buffer, const protocol::PacketType packetType)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		boost::system::error_code error;

		if (!connected && !connecting && !accepting)
		{//Can only send user defined packets in the connected state
			spehs::log::info("SocketTCP: cannot send a packet. Socket is neither connected, connecting nor accepting.");
			return false;
		}
		
		//Codex header
		protocol::WriteBuffer headerBuffer;
		const ExpectedBytesType dataBufferSize = buffer.getOffset();
		const ExpectedBytesType headerBytesValue = buffer.getOffset() + sizeof(packetType);

		headerBuffer.write(headerBytesValue);
		headerBuffer.write(packetType);

		const size_t headerBufferSize = headerBuffer.getOffset();
		size_t offset = 0;
		while (offset < headerBufferSize)
		{//Keep sending data until the whole header has been sent
			offset += socket.write_some(boost::asio::buffer(headerBuffer[offset], headerBufferSize - offset), error);
			if (error)
			{//Error occured while sending data...
				spehs::log::warning("SocketTCP: failed to send packet's codex header! Boost asio error: " + error.message());
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
				spehs::log::warning("SocketTCP: failed to send packet! Boost asio error: " + error.message());
				return false;
			}
		}

		if (debugLogLevel >= 2)
			spehs::log::info("SocketTCP: packet sent. Contents: 4(packet byte size) + 1(packet type) + " + std::to_string(buffer.getOffset()) + "(data)");
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
			spehs::log::info("SocketTCP failed to start receiving. Socket is already receiving.");
			return false;
		}

		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (!connected && !connecting && !accepting)
		{
			spehs::log::warning("SocketTCP: failed to start receiving. Socket is neither connected, connecting nor accepting.");
			return false;
		}
		if (!socket.is_open())
		{//This should never happen as the connected status should guarantee socket being open...
			spehs::log::info("SocketTCP failed to start receiving. Socket has not been opened.");
			return false;
		}

		receiving = true;
		lastReceiveTime = spehs::time::now();
		onReceiveCallback = callbackFunction;
		clearReceivedPackets();
		resumeReceiving();

		if (debugLogLevel >= 2)
		{
			if (expectedBytes > 0)
				spehs::log::info("SocketTCP successfully started receiving. Expecting bytes: " + std::to_string(expectedBytes) + ".");
			else
				spehs::log::info("SocketTCP successfully started receiving. Expecting bytes header.");
		}

		return true;
	}

	void SocketTCP::resumeReceiving()
	{
		if (expectedBytes == 0)
		{//Receive header
			SPEHS_ASSERT(receiveBuffer.size() >= sizeof(expectedBytes));
			boost::asio::async_read(socket, boost::asio::buffer(&receiveBuffer[0], sizeof(expectedBytes)),
				boost::bind(&SocketTCP::receiveHandler,
					this, boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
		else
		{//Receive data
			if (receiveBuffer.size() < expectedBytes)
			{
				if (debugLogLevel >= 1)
					spehs::log::info("SocketTCP: the current receive buffer is insufficient! Expanding to " + std::to_string(expectedBytes));
				receiveBuffer.resize(expectedBytes);
			}
			boost::asio::async_read(socket, boost::asio::buffer(&receiveBuffer[0], expectedBytes),
				boost::bind(&SocketTCP::receiveHandler,
					this, boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
	}

	void SocketTCP::update()
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);

		//onAccept callback
		if (onAcceptCallbackQueued)
		{
			onAcceptCallbackQueued = false;
			onAcceptCallback(*this);
		}

		//Received packets
		std::lock_guard<std::recursive_mutex> lock2(receivedPacketsMutex);
		if (onReceiveCallback)
		{
			for (size_t i = 0; i < receivedPackets.size(); i++)
			{
				codex::protocol::ReadBuffer buffer(receivedPackets[i]->data(), receivedPackets[i]->size());
				onReceiveCallback(buffer);
			}
		}
		clearReceivedPackets();
	}

	void SocketTCP::receiveHandler(const boost::system::error_code& error, std::size_t bytes)
	{
		if (debugLogLevel >= 2)
			spehs::log::info("SocketTCP receive handler received " + std::to_string(bytes) + " bytes.");

		std::lock_guard<std::recursive_mutex> lock(mutex);
		receiving = false;
		lastReceiveTime = spehs::time::now();

		if (debugLogLevel >= 3)
		{
			spehs::log::info("Received bytes: ");
			for (size_t i = 0; i < bytes; i++)
				spehs::log::info("    [" + std::to_string(i) + "] " + spehs::toHexString(receiveBuffer[i]));
		}

		if (error)
		{
			if (error == boost::asio::error::eof)
			{//Connection gracefully closed
				spehs::log::info("SocketTCP disconnected. Remote socket closed connection.");
				disconnect(protocol::DisconnectType::doNotSendDisconnectPacket);
				return;
			}
			else if (error == boost::asio::error::connection_reset)
			{//Disconnect
				spehs::log::info("SocketTCP disconnected. Remote socket closed connection.");
				disconnect(protocol::DisconnectType::doNotSendDisconnectPacket);
				return;
			}
			else if (error == boost::asio::error::connection_aborted ||
				error == boost::asio::error::connection_refused ||
				error == boost::asio::error::bad_descriptor ||
				error == boost::asio::error::operation_aborted)
			{
				spehs::log::info("Boost asio error: " + std::to_string(error.value()));
				if (error == boost::asio::error::connection_aborted)
					spehs::log::info("Closing client: boost asio error: connection_aborted");
				if (error == boost::asio::error::connection_refused)
					spehs::log::info("Closing client: boost asio error: connection_refused");
				if (error == boost::asio::error::bad_descriptor)
					spehs::log::info("Closing client: boost asio error: bad_descriptor");
				if (error == boost::asio::error::operation_aborted)
					spehs::log::info("Closing client: boost asio error: operation_aborted");
				disconnect(protocol::DisconnectType::doNotSendDisconnectPacket);
				return;
			}
			else
			{//Ignored error
				spehs::log::info("Ignored ASIO error: " + std::to_string(error.value()) + ": " + error.message());
			}
		}

		if (bytes)
		{
			protocol::ReadBuffer readBuffer(&receiveBuffer[0], bytes);
			if (expectedBytes == 0)
			{//Header received
				readBuffer.read(expectedBytes);
				resumeReceiving();
			}
			else if (expectedBytes == bytes)
			{//Data received

				//Read buffer
				const bool keepReceiving = codexReceiveHandler(readBuffer);
				expectedBytes = 0;//Begin to expect header next
				if (keepReceiving)
					resumeReceiving();
			}
			else
			{
				spehs::log::warning("SocketTCP received unexpected amount of bytes! Canceling receiving!");
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
			spehs::log::info("SocketTCP codex receive handler received packet of type: " + std::to_string((int)packetType) + ", " + std::to_string(buffer.getBytesRemaining()) + " bytes.");

		//Process packet
		switch (packetType)
		{
		case protocol::PacketType::undefined:
		{
			const size_t userBytes = buffer.getBytesRemaining();
			if (debugLogLevel >= 2)
				spehs::log::info("SocketTCP received user defined packet. Bytes: " + std::to_string(userBytes));

			if (onReceiveCallback)
			{
				//Push to received packets queue
				std::lock_guard<std::recursive_mutex> lock2(receivedPacketsMutex);
				receivedPackets.push_back(new std::vector<uint8_t>(userBytes));
				memcpy(receivedPackets.back()->data(), buffer[buffer.getOffset()], userBytes);
				return true;
			}
			else
			{
				spehs::log::warning("SocketTCP::codexReceiveHandler: no receive handler provided for the user defined packet.");
				return false;
			}
		}
		case protocol::PacketType::disconnect:
		{
			protocol::DisconnectType disconnectType;
			buffer.read(disconnectType);
			disconnect(protocol::DisconnectType::doNotSendDisconnectPacket);
			spehs::log::info("SocketTCP: remote socket gracefully disconnected. Disconnect type was: " + std::to_string((uint8_t)disconnectType));
			return false;
		}
		case protocol::PacketType::handshake:
		{
			if (handshakeReceived)
			{
				spehs::log::warning("A handshake for the current connection has already been received!");
				return false;
			}
			protocol::Handshake handshake;
			buffer.read(handshake);

			if (handshake.isValid())
			{//VALID HANDSHAKE
				handshakeReceived = true;
				if (debugLogLevel >= 1)
					spehs::log::info("SocketTCP valid handshake received.");
			}
			else
			{//INVALID HANDSHAKE -> DISCARD
				handshakeReceived = true;
				spehs::log::warning("Received an invalid codex handshake!");
			}
			return false;
		}
		}
	}

	bool SocketTCP::startAccepting(const protocol::PortType port, const std::function<void(SocketTCP&)> callbackFunction)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (accepting)
		{
			spehs::log::info("SocketTCP failed to start accepting! Socket is already accepting!");
			return false;
		}
		if (connected)
		{
			spehs::log::warning("SocketTCP failed start accepting! Socket is currently connected!");
			return false;
		}
		SPEHS_ASSERT(!handshakeSent);
		SPEHS_ASSERT(!handshakeReceived);
		onAcceptCallbackQueued = false;

		if (acceptor == nullptr)
			acceptor = new boost::asio::ip::tcp::acceptor(ioService.getImplementationRef());
		boost::system::error_code error;
		const boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);

		//Open acceptor
		acceptor->open(endpoint.protocol(), error);
		if (error)
		{
			spehs::log::warning("Failed to open acceptor! Boost asio error: " + error.message());
			return false;
		}

		//Bind acceptor
		acceptor->bind(endpoint, error);
		if (error)
		{
			spehs::log::warning("Failed to bind acceptor! Boost asio error: " + error.message());
			return false;
		}
		if (!acceptor->is_open())
		{
			spehs::log::warning("Boost acceptor failed to open!");
			return false;
		}

		//Make acceptor listen
		acceptor->listen(boost::asio::socket_base::max_connections, error);
		if (error)
		{
			spehs::log::warning("Failed to make acceptor listen! Boost asio error: " + error.message());
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
		SPEHS_ASSERT(!isConnected());
		SPEHS_ASSERT(!isReceiving());
		SPEHS_ASSERT(isAccepting());
		SPEHS_ASSERT(acceptor);
		SPEHS_ASSERT(!codexAcceptThread);
		acceptor->close();
		if (error)
		{
			spehs::log::warning("SocketTCP failed to accept an incoming connection! Boost asio error: " + error.message() + "Accepting has stopped.");
			disconnect(protocol::DisconnectType::doNotSendDisconnectPacket);
			accepting = false;
			return;
		}
		else
		{
			codexAcceptThread = new std::thread(&SocketTCP::codexAccept, this);
		}
	}

	void SocketTCP::codexAccept()
	{
		//Wait until received remote handshake
		if (debugLogLevel >= 1)
			spehs::log::info("Accepting SocketTCP expecting a handshake...");

		//Start expecting an incoming handshake (connector sends first)
		startReceiving(onReceiveCallback);
		waitUntilReceivedHandshake(handshakeReceiveTimeout);

		{//Check if received the handshake in time
			std::lock_guard<std::recursive_mutex> lock(mutex);
			if (!handshakeReceived)
			{
				spehs::log::info("SocketTCP failed to accept an incoming connection! No response handshake received!");
				disconnect(protocol::DisconnectType::doNotSendDisconnectPacket);
				onAcceptCallbackQueued = true;
				return;
			}
		}
		if (debugLogLevel >= 1)
			spehs::log::info("Accepting SocketTCP received a handshake.");
		
		//Send a response handshake
		protocol::WriteBuffer buffer;
		const protocol::Handshake handshake;
		buffer.write(handshake);
		if (sendPacket(buffer, protocol::PacketType::handshake))
		{
			handshakeSent = true;
			if (debugLogLevel >= 1)
				spehs::log::info("Accepting SocketTCP sent a handshake.");
		}
		else
		{
			spehs::log::info("SocketTCP failed to accept an incoming connection! Could not send handshake!");
			disconnect(protocol::DisconnectType::doNotSendDisconnectPacket);

			std::lock_guard<std::recursive_mutex> lock(mutex);
			onAcceptCallbackQueued = true;
			accepting = false;
			return;
		}

		//Socket is now in the connected state!
		std::lock_guard<std::recursive_mutex> lock(mutex);
		accepting = false;
		connected = true;
		lastReceiveTime = spehs::time::now();
		onAcceptCallbackQueued = true;
	}

	std::string SocketTCP::getRemoteAddress() const
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (!isConnected())
		{
			spehs::log::info("SocketTCP: cannot retrieve remote endpoint address! Socket is not connected!");
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
			spehs::log::info("SocketTCP: cannot retrieve remote endpoint port! Socket is not connected!");
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
			spehs::log::info("SocketTCP: cannot retrieve remote endpoint port! Socket is not connected!");
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
		if (spehs::time::now() - lastReceiveTime >= connectionTimeout)
			return false;
		return connected;
	}
//		std::string ghostName;
//		buffer.read(ghostName);
//		const std::string ghostPath = workingDirectory + "/" + ghostName + ".exe";
//		if (fileExists(ghostPath))
//		{//Ghost file exists
//			//Search for the specified ghost program on local drive. If ghost program exists, launch it and pass it the remote endpoint to connect to.
//
//#ifdef _WIN32
//			// additional information
//			STARTUPINFO si;
//			PROCESS_INFORMATION pi;
//			// set the size of the structures
//			ZeroMemory(&si, sizeof(si));
//			si.cb = sizeof(si);
//			ZeroMemory(&pi, sizeof(pi));
//			// start the program up
//			CreateProcess(ghostPath.c_str(), // format: "E:/Ohjelmointi/Projects/Aria/Solution/bin/Ghost0.exe"
//				"param0",		// Command line
//				NULL,           // Process handle not inheritable
//				NULL,           // Thread handle not inheritable
//				FALSE,          // Set handle inheritance to FALSE
//				CREATE_NEW_CONSOLE,//Creation flags
//				NULL,           // Use parent's environment block
//				NULL,           // Use parent's starting directory 
//				&si,            // Pointer to STARTUPINFO structure
//				&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
//			);
//			// Close process and thread handles.
//			CloseHandle(pi.hProcess);
//			CloseHandle(pi.hThread);
//#elif
//#warning Missing implementation for ghost launching on current platform
//#endif
}