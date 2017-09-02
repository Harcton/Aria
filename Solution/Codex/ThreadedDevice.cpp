#include "ThreadedDevice.h"
#include <assert.h>
#include "Log.h"


namespace codex
{
	ThreadedDevice::ThreadedDevice()
		: thread(nullptr)
		, keepRunning(false)
	{
	}

	ThreadedDevice::~ThreadedDevice()
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (thread)
		{
			log::error("ThreadedDevice was not stopped before destructing! ThreadDevices must always be manually stopped before destructing!");
			assert(false);
		}
	}

	bool ThreadedDevice::start()
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (thread)
		{
			log::info("Threaded device has already been started!");
			return false;
		}

		//Make onStart() call, device can prepare for running.
		log::info("Starting threaded device...");
		onStart();

		//Launch run thread
		keepRunning = true;
		thread = new std::thread(&ThreadedDevice::run, this);

		return true;
	}

	bool ThreadedDevice::stop()
	{
		{std::lock_guard<std::recursive_mutex> lock(mutex);
			if (!thread)
				return false;

			//Request the run thread to exit
			keepRunning = false;
		}

		//Wait until the run thread finishes execution, then deallocate thread. NOTE: no mutex acquisition, let the run thread have it...
		thread->join();
		std::lock_guard<std::recursive_mutex> lock(mutex);
		delete thread;
		thread = nullptr;

		//Make onStop() call, device can do post-run operations.
		onStop();

		return true;
	}

	void ThreadedDevice::run()
	{
		//Update until stop is requested
		log::info("Running threaded device...");
		bool _keepRunning = true;
		while (_keepRunning)
		{
			update();

			std::lock_guard<std::recursive_mutex> lock(mutex);
			_keepRunning = keepRunning;
		}
	}

	bool ThreadedDevice::isRunning() const
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		return thread != nullptr;
	}
}