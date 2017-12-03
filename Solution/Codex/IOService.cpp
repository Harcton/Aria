#include "IOService.h"
#include "Log.h"
#include <boost/bind.hpp>


namespace codex
{
	IOService::IOService()
		: io_service()
		, work(io_service)
		, thread(boost::bind(&IOService::run, this))
	{		

	}
	
	IOService::~IOService()
	{
		try
		{
			io_service.stop();
		}
		catch (std::exception& e)
		{
			log::info(e.what());
		}
		thread.join();
	}

	void IOService::run()
	{
		try
		{
			io_service.run();
		}
		catch (std::exception& e)
		{
			log::info(e.what());
		}
	}

	boost::asio::io_service& IOService::getImplementationRef()
	{
		return io_service;
	}
}