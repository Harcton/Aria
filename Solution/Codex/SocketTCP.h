#pragma once
#include <mutex>
#include <thread>
#include <functional>
#include <type_traits>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace codex
{
	/** Intended to be used by Shell programs */
	class SocketTCP
	{
	public:
		typedef uint16_t packet_size;
	public:
		SocketTCP(const packet_size receiveBufferSize);
		~SocketTCP();
		
		bool connect(const char* address_ipv4, const packet_size port);
		void disconnect();
		bool sendPacket(const unsigned char* data, const packet_size bytes);
		/* Returns false if the memory allocation fails, or the socket is currently receiving data. */
		bool resizeReceiveBuffer(const packet_size newSize);
		bool startReceiving(const std::function<void(const unsigned char*, packet_size)> callbackFunction);
		void receiveHandler(const boost::system::error_code error, const packet_size bytes);

		boost::asio::ip::tcp::endpoint getRemoteEndpoint() const { return remoteEndpoint; }

	private:
		boost::asio::io_service ioService;
		std::thread* ioServiceThread;
		boost::asio::ip::tcp::socket socket;
		boost::asio::ip::tcp::endpoint remoteEndpoint;

		std::recursive_mutex receiveMutex;
		std::function<void(const unsigned char*, packet_size)> receiveHandlerCallback;
		packet_size expectedBytes;
		unsigned char* receiveBuffer;
		packet_size receiveBufferSize;
		bool isReceiving;
	};
}