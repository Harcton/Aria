#pragma once
#include <thread>
#include <mutex>
#include "CodexTime.h"

namespace codex
{
	namespace device
	{
		/*
			Derived class must consider that the update call is made from a non-main thread.
		*/
		class ThreadedDevice
		{
		public:
			ThreadedDevice();
			virtual ~ThreadedDevice() = 0;

			/*
				Upon returning, the device is guaranteed to have started running in a different thread.
				Furthermore, the device may possibly have even stopped before start() returns.
			*/
			bool start();
			/* Asynchronously stops the device */
			void stop();
			bool isRunning() const;

		protected:
			virtual void onStart() {}
			virtual void update() = 0;
			virtual void onStop() {}
		private:
			void run();
			mutable std::recursive_mutex mutex;
			std::thread* thread;
			bool keepRunning;
			bool threadRunning;
			bool canExitStart;//Used to prevent start() from returning before run() has successfully started in a different thread.
		};
	}
}