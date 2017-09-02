#pragma once
#include <thread>
#include <mutex>
#include "CodexTime.h"

namespace codex
{
	/*
		Derived class must consider that the update call is made from a non-main thread.
	*/
	class ThreadedDevice
	{
	public:
		ThreadedDevice();
		virtual ~ThreadedDevice();

		bool start();
		bool stop();
		bool isRunning() const;

	protected:
		virtual void onStart() {}
		virtual void update() = 0;
		virtual void onStop(){}
	private:
		void run();
		mutable std::recursive_mutex mutex;
		std::thread* thread;
		bool keepRunning;
	};
}