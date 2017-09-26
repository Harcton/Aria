#include "ThreadedDevice.h"
#include "RAIIVariableSetter.h"
#include <assert.h>
#include "Log.h"


namespace codex
{
	namespace device
	{
		ThreadedDevice::ThreadedDevice()
			: thread(nullptr)
			, keepRunning(false)
			, threadRunning(false)
		{
		}

		ThreadedDevice::~ThreadedDevice()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			assert(!keepRunning);//Stop must have been requested by now!
			if (thread)
			{
				thread->join();
				delete thread;
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

			log::info("Starting threaded device...");

			//Launch run thread
			keepRunning = true;
			thread = new std::thread(&ThreadedDevice::run, this);

			return true;
		}

		bool ThreadedDevice::stop()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			if (!thread || !keepRunning)
				return false;

			//Request the run thread to exit
			keepRunning = false;
			return true;
		}

		void ThreadedDevice::run()
		{
			RAIIMutexVariableSetter<bool, std::recursive_mutex> threadRunningSetter(threadRunning, true, mutex);

			//Make onStart() call, device can prepare for running.
			onStart();

			//Update until stop is requested
			while (true)
			{
				update();
				std::lock_guard<std::recursive_mutex> lock(mutex);
				if (!keepRunning)
					break;
			}

			//Make onStop() call, device can do post-run operations.
			onStop();
		}

		bool ThreadedDevice::isRunning() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return threadRunning;
		}
	}
}