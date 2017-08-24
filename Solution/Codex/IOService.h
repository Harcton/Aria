#pragma once
#include <boost/asio/io_service.hpp>
#include <thread>

namespace codex
{
	/*Upon creation, starts to run an IO service in a separate thread*/
	class IOService
	{
	public:
		IOService();
		~IOService();

		boost::asio::io_service& getImplementationRef();

	private:
		boost::asio::io_service implementation;
		boost::asio::io_service::work work;
		std::thread thread;
	};
}