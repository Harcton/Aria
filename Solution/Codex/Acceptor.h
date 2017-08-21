#pragma once
#include <thread>
#include <stdint.h>
#include <boost/asio.hpp>

namespace codex
{
	class SocketTCP;
	class Acceptor
	{
	public:
		friend class SocketTCP;
	public:
		Acceptor();
		~Acceptor();

		bool open(const uint16_t _port);

	private:
		uint16_t port;
		boost::asio::io_service ioService;
		boost::asio::io_service::work ioServiceWork;
		std::thread ioServiceThread;
		boost::asio::ip::tcp::acceptor acceptor;
	};
}