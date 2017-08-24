#pragma once
#include <thread>
#include <mutex>
#include "GPIO.h"


namespace codex
{
	class DCMotor
	{
	public:
		DCMotor();
		~DCMotor();

		void run();
		void stop();
		void runLoop();

		void setPins(const gpio::Pin pulseWidthPin, const gpio::Pin inputPin1, gpio::Pin inputPin2);

	private:
		mutable std::recursive_mutex mutex;
		std::thread* runThread;
		bool stopRequested;
		gpio::Pin pulseWidthPin;
		gpio::Pin inputPin1;
		gpio::Pin inputPin2;
	};
}