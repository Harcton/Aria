#pragma once
#include <mutex>
#include <thread>
#include <functional>
#include <type_traits>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "Protocol.h"


namespace codex
{
	/** Intended to be used by Shell programs */
	class SocketTCP
	{
	public:
		SocketTCP();
		~SocketTCP();
		
		bool connect(const char* address_ipv4, const uint16_t port);
		void disconnect();
		bool sendPacket(const protocol::WriteBuffer& buffer);
		/* Returns false if the memory allocation fails, or the socket is currently receiving data. */
		bool resizeReceiveBuffer(const size_t newSize);
		bool startReceiving(const std::function<void(const void*, size_t)> callbackFunction);
		void receiveHandler(const boost::system::error_code error, const size_t bytes);

		boost::asio::ip::tcp::endpoint getRemoteEndpoint() const { return remoteEndpoint; }

	private:
		boost::asio::io_service ioService;
		std::thread* ioServiceThread;
		boost::asio::ip::tcp::socket socket;
		boost::asio::ip::tcp::endpoint remoteEndpoint;

		std::recursive_mutex receiveMutex;
		std::function<void(const void*, size_t)> receiveHandlerCallback;
		size_t expectedBytes;
		unsigned char* receiveBuffer;
		size_t receiveBufferSize;
		bool isReceiving;
	private:
		typedef uint32_t PacketHeaderBytesType;
	};
}