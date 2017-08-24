#include "IOService.h"
#include <boost/bind.hpp>


namespace codex
{
	IOService::IOService()
		: implementation()
		, work(implementation)
		, thread(boost::bind(&boost::asio::io_service::run, &implementation))
	{
	}
	
	IOService::~IOService()
	{
		implementation.stop();
		thread.join();
	}

	boost::asio::io_service& IOService::getImplementationRef()
	{
		return implementation;
	}

}