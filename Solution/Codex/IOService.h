#pragma once
#include <thread>
#include <boost/asio/io_service.hpp>

namespace codex
{
	/*Upon creation, starts to run an IO service in a separate thread*/
	class IOService
	{
		friend class SocketTCP;
	public:
		IOService();
		~IOService();

	private:
		boost::asio::io_service ioService;
		boost::asio::io_service::work work;
		std::thread thread;
	};
}