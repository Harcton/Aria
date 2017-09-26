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

			bool start();
			/* Asynchronously stops the device */
			bool stop();
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
		};
	}
}