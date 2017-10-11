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
			assert(!keepRunning);//Stop must have been requested by now! -> do it in the overriding destructor...
			if (thread)
			{
				thread->join();
				delete thread;
			}
		}

		bool ThreadedDevice::start()
		{
			{
				std::lock_guard<std::recursive_mutex> lock(mutex);

				if (isRunning())
				{
					log::info("Threaded device is already running!");
					return false;
				}

				if (thread)
				{//Wait for thread to fully exit
					thread->join();
					delete thread;
					thread = nullptr;
				}

				log::info("Starting threaded device...");

				//Launch run thread
				keepRunning = true;
				canExitStart = false;
				thread = new std::thread(&ThreadedDevice::run, this);
			}

			//Blocks until run has started in a different thread
			while (true)
			{
				std::lock_guard<std::recursive_mutex> lock(mutex);
				if (canExitStart)
					break;
			}

			return true;
		}

		void ThreadedDevice::stop()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			if (!thread || !keepRunning)
				return;

			//Request the run thread to exit
			keepRunning = false;
			return;
		}

		void ThreadedDevice::run()
		{
			RAIIMutexVariableSetter<bool, std::recursive_mutex> threadRunningSetter(threadRunning, true, mutex);

			//Notify start() to be able to return
			{
				std::lock_guard<std::recursive_mutex> lock(mutex);
				canExitStart = true;
			}

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