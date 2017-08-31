#include "IOService.h"
#include <boost/bind.hpp>


namespace codex
{
	IOService::IOService()
		: io_service()
		, work(io_service)
		, thread(boost::bind(&boost::asio::io_service::run, &io_service))
	{
	}
	
	IOService::~IOService()
	{
		io_service.stop();
		thread.join();
	}

	boost::asio::io_service& IOService::getImplementationRef()
	{
		return io_service;
	}
}