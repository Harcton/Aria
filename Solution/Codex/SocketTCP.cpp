//#include <assert.h>
#include <iostream>
#include <string>
#include "SocketTCP.h"
#include "Acceptor.h"
#include "IOService.h"
#include "Log.h"
#include "CodexTime.h"


namespace codex
{
	SocketTCP::SocketTCP(IOService& _ioService)
		: ioService(_ioService)
		, socket(_ioService.getImplementationRef())
		, acceptor(nullptr)
		, onReceiveCallback()
		, onAcceptCallback()
		, expectedBytes(0)
		, receiveBuffer(sizeof(size_t))
		, receiving(false)
		, connected(false)
		, reverseByteOrdering(false)
	{
	}
	SocketTCP::~SocketTCP()
	{
		disconnect();
		stopAccepting();
		waitUntilFinishedReceiving();
	}

	bool SocketTCP::waitUntilFinishedReceiving()
	{
		bool wait = true;
		while (wait)
		{
			mutex.lock();
			wait = receiving;
			mutex.unlock();
		}
		return true;
	}

	bool SocketTCP::waitUntilFinishedReceiving(uint64_t millisecondTimeout)
	{
		bool wait = true;
		while (wait)
		{
			mutex.lock();
			wait = receiving;
			mutex.unlock();
			time::delay(time::milliseconds(1.0f));
			if (--millisecondTimeout == 0)
				return false;
		}
		return true;
	}

	bool SocketTCP::connect(const char* address_ipv4, uint16_t port)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		boost::system::error_code error;
		boost::asio::ip::tcp::resolver resolverTCP(ioService.getImplementationRef());
		boost::asio::ip::tcp::resolver::query query(address_ipv4, std::to_string(port));
		boost::asio::ip::tcp::endpoint endpoint = *resolverTCP.resolve(query, error);
		if (error)
		{
			log::info("SocketTCP::connect() failed to resolve the endpoint. Boost asio error: " + error.message());
			return false;
		}

		socket.connect(endpoint, error);
		if (error)
		{
			log::info("SocketTCP::connect() failed to connect. Boost asio error: " + error.message());
			return false;
		}

		//Finish the connection with an aria handshake
		protocol::WriteBuffer buffer(protocol::Endianness::inverted);
		protocol::aria::Handshake handshake;
		handshake.write(buffer);
		
		connected = true;//Set as connected, so that sendPacket can pass
		return connected = sendPacket(buffer);//If sending the handshake fails, lose the connected status.
	}

	void SocketTCP::disconnect()
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (socket.is_open())
		{
			socket.shutdown(boost::asio::socket_base::shutdown_both);
			socket.close();
		}
		connected = false;
	}

	void SocketTCP::stopAccepting()
	{
		if (acceptor)
		{
			acceptor->cancel();
			acceptor->close();
			delete acceptor;
			acceptor = nullptr;
		}
	}

	bool SocketTCP::sendPacket(const protocol::WriteBuffer& buffer)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		boost::system::error_code error;

		if (!connected)
		{
			codex::log::warning("SocketTCP: cannot send a packet! Socket is not connected!");
			return false;
		}

		//if (buffer.hasReversedByteOrder() != reverseByteOrdering)
			//codex::log::warning("SocketTCP byte ordering different from passed WriteBuffer's byte order!");

		//Header
		const ExpectedBytesType bytes = buffer.getWrittenSize();
		if (socket.send(boost::asio::buffer(&bytes, sizeof(bytes)), 0, error) != sizeof(bytes))
		{
			codex::log::warning("SocketTCP: failed to send packet! Bytes header failed to send! " + (error ? "Boost asio error : " + error.message() : ""));
			return false;
		}
		if (error)
		{//Error occured while sending the header...
			codex::log::warning("SocketTCP: failed to send packet! Boost asio error: " + error.message());
			return false;
		}

		//Data
		size_t offset = 0;
		while (offset < bytes)
		{//Keep sending data until all data has been sent
			offset += socket.write_some(boost::asio::buffer(buffer[offset], bytes - offset), error);
			if (error)
			{//Error occured while sending data...
				codex::log::warning("SocketTCP: failed to send packet! Boost asio error: " + error.message());
				return false;
			}
		}

		return true;
	}

	bool SocketTCP::resizeReceiveBuffer(const size_t newSize)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (receiving)
			return false;
		receiveBuffer.resize(newSize);
		return true;
	}

	bool SocketTCP::startReceiving(const std::function<bool(protocol::ReadBuffer&)> callbackFunction)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (!socket.is_open())
		{
			log::info("SocketTCP failed to start receiving. Socket has not been connected!");
			return false;
		}
		if (receiveBuffer.size() == 0)
		{
			log::info("SocketTCP failed to start receiving. Receive buffer size is set to 0!");
			return false;
		}
		if (!connected)
		{
			codex::log::warning("SocketTCP: cannot start receiving! Socket is not connected!");
			return false;
		}

		receiving = true;
		onReceiveCallback = callbackFunction;
		if (expectedBytes)
		{//Receive data
			boost::asio::async_read(socket, boost::asio::buffer(&receiveBuffer[0], expectedBytes),
				boost::bind(&SocketTCP::receiveHandler,
					this, boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
		else
		{//Receive header
			boost::asio::async_read(socket, boost::asio::buffer(&receiveBuffer[0], sizeof(expectedBytes)),
				boost::bind(&SocketTCP::receiveHandler,
					this, boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
		return true;
	}
	
	bool SocketTCP::handshakeReceiveHandler(codex::protocol::ReadBuffer& buffer)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);

		protocol::aria::Handshake handshake;
		handshake.read(buffer);
		if (handshake.isValid())
		{//VALID HANDSHAKE

			mutex.lock();
			 //Deallocate acceptor
			delete acceptor;
			acceptor = nullptr;
			//Set byte endianness
			reverseByteOrdering = handshake.getEndianness() == protocol::Endianness::inverted;
			mutex.unlock();

			onAcceptCallback(*this);
		}
		else
		{//INVALID HANDSHAKE -> DISCARD
			log::error("Received invalid aria handshake! Resuming accepting another connection.");

			//Deallocate acceptor
			mutex.lock();
			const uint16_t port = acceptor->local_endpoint().port();
			//assert(acceptor);
			delete acceptor;
			acceptor = nullptr;
			mutex.unlock();

			//Disconnect from current endpoint as it gave no aria handshake
			disconnect();

			//Resume accepting...
			startAccepting(port, onAcceptCallback);
		}
		return false;//Do not start receiving again!
	}

	void SocketTCP::receiveHandler(const boost::system::error_code& error, std::size_t bytes)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);

		if (error)
		{
			if (error == boost::asio::error::eof)
			{//Connection gracefully closed
				log::info("SocketTCP closed.");
				receiving = false;
				connected = false;
				return;
			}
			else if (error == boost::asio::error::connection_reset)
			{//Disconnect
				log::info("SocketTCP disconnected.");
				receiving = false;
				connected = false;
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
				receiving = false;
				disconnect();
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
				memcpy(&expectedBytes, receiveBuffer.data(), sizeof(expectedBytes));
				startReceiving(onReceiveCallback);
			}
			else if (expectedBytes == bytes)
			{//Data received
				//receiving == true! prevent buffer resize from within the callback!
				protocol::ReadBuffer readBuffer(receiveBuffer.data(), bytes);
				const bool keepReceiving = onReceiveCallback(readBuffer);
				expectedBytes = 0;//Begin to expect header next
				if (keepReceiving)
					startReceiving(onReceiveCallback);
				else
					receiving = false;
			}
			else
			{
				log::warning("SocketTCP received unexpected amount of bytes! Canceling receiving!");
			}
		}
	}

	bool SocketTCP::startAccepting(const uint16_t port, const std::function<void(SocketTCP&)> callbackFunction)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (acceptor)
		{
			log::info("SocketTCP failed to start accepting! Accepting has already been started!");
			return false;
		}
		if (connected)
		{
			codex::log::warning("SocketTCP: cannot start accepting! Socket is already connected!");
			return false;
		}

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
		//acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true)); ?

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

		//Start accepting asynchoronously
		acceptor->async_accept(socket, boost::bind(&SocketTCP::onAccept, this, boost::asio::placeholders::error));

		//Set the callback function
		onAcceptCallback = callbackFunction;

		return true;
	}

	void SocketTCP::onAccept(const boost::system::error_code error)
	{
		if (error)
		{
			log::warning("SocketTCP failed to accept an incoming connection! Boost asio error: " + error.message() + "Accepting has stopped.");
			return;
		}
		else
			connected = true;
		
		//Finish accepting by getting the aria handshake...
		startReceiving(std::bind(&SocketTCP::handshakeReceiveHandler, this, std::placeholders::_1));
	}

	std::string SocketTCP::getRemoteAddress()
	{
		if (!connected)
		{
			codex::log::warning("SocketTCP: cannot retrieve remote access! Socket is not connected!");
			return "0.0.0.0";
		}
		boost::asio::ip::tcp::endpoint endpoint = socket.remote_endpoint();
		boost::asio::ip::address address = endpoint.address();
		boost::asio::ip::address_v4 address_v4 = address.to_v4();
		return address_v4.to_string();
	}
}