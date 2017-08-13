#include <iostream>
#include <string>
#include "SocketTCP.h"
#include "Log.h"


namespace codex
{
	SocketTCP::SocketTCP() : ioService(), socket(ioService), expectedBytes(0), receiveBuffer(nullptr), receiveBufferSize(0), isReceiving(false)
	{
		//Start asio io service
		boost::asio::io_service::work work(ioService);
		ioServiceThread = new std::thread(boost::bind(&boost::asio::io_service::run, &ioService));
	}
	SocketTCP::~SocketTCP()
	{
		disconnect();

		//Stop asio io service
		ioService.stop();
		ioServiceThread->join();
		delete ioServiceThread;

		//Wait until finished receiving
		bool wait = true;
		while (wait)
		{
			receiveMutex.lock();
			wait = isReceiving;
			receiveMutex.unlock();
		}

		//Deallocate receive buffer
		receiveMutex.lock();
		delete[] receiveBuffer;
		receiveMutex.unlock();
	}

	bool SocketTCP::connect(const char* address_ipv4, uint16_t port)
	{
		boost::system::error_code error;
		boost::asio::ip::tcp::resolver resolverTCP(ioService);
		boost::asio::ip::address_v4 address = boost::asio::ip::address_v4::from_string(address_ipv4);
		boost::asio::ip::tcp::resolver::query query(address_ipv4, std::to_string(port));

		remoteEndpoint = *resolverTCP.resolve(query, error);
		if (error)
			return false;

		socket.connect(remoteEndpoint, error);
		if (error)
			return false;

		return true;
	}

	void SocketTCP::disconnect()
	{
		if (socket.is_open())
		{
			socket.shutdown(boost::asio::socket_base::shutdown_both);
			socket.close();
		}
	}

	bool SocketTCP::sendPacket(const protocol::WriteBuffer& buffer)
	{
		boost::system::error_code error;		

		//Header
		const PacketHeaderBytesType bytes = buffer.getSize();
		socket.send(boost::asio::buffer(&bytes, 2), 0, error);
		if (error)
		{//Error occured while sending the header...
			return false;
		}

		//Data
		size_t offset = 0;
		size_t sent = 0;
		while (sent < bytes)
		{//Keep sending data until all data has been sent
			sent = socket.write_some(boost::asio::buffer(buffer[offset], bytes - sent), error);
			if (error)
			{//Error occured while sending data...
				return false;
			}
		}

		return true;
	}

	bool SocketTCP::resizeReceiveBuffer(const size_t newSize)
	{
		std::lock_guard<std::recursive_mutex> lock(receiveMutex);
		if (isReceiving)
			return false;

		if (receiveBufferSize == newSize)
			return false;
		if (receiveBuffer)
			delete[] receiveBuffer;
		if (newSize > 0)
		{
			receiveBuffer = new unsigned char[newSize];
			if (receiveBuffer)
			{
				receiveBufferSize = newSize;
				return true;
			}
			else
			{
				receiveBufferSize = 0;
				return false;
			}
		}
		else
		{
			receiveBuffer = nullptr;
			receiveBufferSize = 0;
			return true;
		}
	}

	bool SocketTCP::startReceiving(const std::function<void(const void*, size_t)> callbackFunction)
	{
		std::lock_guard<std::recursive_mutex> lock(receiveMutex);
		if (socket.is_open() && receiveBufferSize > 0)
		{
			isReceiving = true;
			receiveHandlerCallback = callbackFunction;
			if (expectedBytes)
			{//Receive data
				boost::asio::async_read(socket, boost::asio::buffer(receiveBuffer, expectedBytes),
					boost::bind(&SocketTCP::receiveHandler,
						this, boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
			}
			else
			{//Receive header
				boost::asio::async_read(socket, boost::asio::buffer(receiveBuffer, sizeof(expectedBytes)),
					boost::bind(&SocketTCP::receiveHandler,
						this, boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
			}
			return true;
		}
		else
			return false;
	}
	
	void SocketTCP::receiveHandler(const boost::system::error_code error, const size_t bytes)
	{
		std::lock_guard<std::recursive_mutex> lock(receiveMutex);
		isReceiving = false;

		if (error)
		{
			if (error == boost::asio::error::eof)
			{//Connection gracefully closed
				return;
			}
			else if (error == boost::asio::error::connection_reset)
			{//Disconnect
				return;
			}
			else if (error == boost::asio::error::connection_aborted ||
				error == boost::asio::error::connection_refused ||
				error == boost::asio::error::bad_descriptor ||
				error == boost::asio::error::operation_aborted)
			{
				std::cout << "\nBoost asio error: " + std::to_string(error.value());
				if (error == boost::asio::error::connection_aborted) std::cout << "\n\tClosing client: boost asio error: connection_aborted";
				if (error == boost::asio::error::connection_refused) std::cout << "\n\tClosing client: boost asio error: connection_refused";
				if (error == boost::asio::error::bad_descriptor) std::cout << "\n\tClosing client: boost asio error: bad_descriptor";
				if (error == boost::asio::error::operation_aborted) std::cout << "\n\tClosing client: boost asio error: operation_aborted";
				return;
			}
			else
			{//Ignored error
				std::cout << "\nIgnored ASIO error: " + std::to_string(error.value()) + ": " + error.message();
			}

			startReceiving(receiveHandlerCallback);
			return;
		}

		if (bytes)
		{
			if (expectedBytes == 0)
			{//Header received
				memcpy(&expectedBytes, receiveBuffer, sizeof(expectedBytes));
			}
			else
			{//Data received
				isReceiving = true;//! prevent buffer resize from within the callback!
				receiveHandlerCallback(receiveBuffer, expectedBytes);
				isReceiving = false;//!
				expectedBytes = 0;//Begin to expect header next
			}
		}

		startReceiving(receiveHandlerCallback);
	}
}