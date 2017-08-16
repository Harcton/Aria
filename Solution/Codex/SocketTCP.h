#pragma once
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <mutex>
#include <thread>
#include <functional>
#include <type_traits>
#include "Protocol.h"

namespace codex
{
	class Acceptor;

	class SocketTCP
	{
	public:
		SocketTCP();
		~SocketTCP();
		
		/* Perform a synchronous connection attempt. */
		bool connect(const char* address_ipv4, const uint16_t port);
		void disconnect();
		bool sendPacket(const protocol::WriteBuffer& buffer);
		/* Returns false if the memory allocation fails, or the socket is currently receiving data. */
		bool resizeReceiveBuffer(const size_t newSize);
		bool startReceiving(const std::function<void(const void*, size_t)> callbackFunction);
		void receiveHandler(const boost::system::error_code error, const size_t bytes);
		void startAccepting(Acceptor& acceptor, const std::function<void(bool, SocketTCP&)> callbackFunction);

		boost::asio::ip::tcp::endpoint getRemoteEndpoint() const { return remoteEndpoint; }
		
		void onAccept(const boost::system::error_code error);

	private:

		boost::asio::io_service ioService;
		std::thread* ioServiceThread;
		boost::asio::ip::tcp::socket socket;
		boost::asio::ip::tcp::endpoint remoteEndpoint;

		std::recursive_mutex receiveMutex;
		std::function<void(const void*, size_t)> receiveHandlerCallback;
		std::function<void(bool, SocketTCP&)> onAcceptCallback;
		size_t expectedBytes;
		unsigned char* receiveBuffer;
		size_t receiveBufferSize;
		bool isReceiving;
	private:
		typedef uint32_t PacketHeaderBytesType;
	};
}