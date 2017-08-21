#include "IOService.h"
#include <boost/bind.hpp>


namespace codex
{
	IOService::IOService()
		: ioService()
		, work(ioService)
		, thread(boost::bind(&boost::asio::io_service::run, &ioService))
	{
	}
	
	IOService::~IOService()
	{
		ioService.stop();
		thread.join();
	}
}