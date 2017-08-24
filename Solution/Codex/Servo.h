#pragma once
#include <thread>
#include <mutex>
#include <iostream>
#include <bcm2835.h>
#include "GPIO.h"
#include "CodexTime.h"
#include "CodexAssert.h"


namespace codex
{
	class Servo
	{
	public:
		Servo();
		~Servo();

		void run();
		void runLoop();
		void stop();

		void setPosition(const unsigned char _position);
		void setPin(const gpio::Pin _pin);
		void mapPosition(const time::TimeType minPulseWidth, const time::TimeType maxPulseWidth);

		gpio::Pin getPin() const;
		unsigned char getPosition() const;
		bool isRunning() const;
		
	private:

		mutable std::recursive_mutex mutex;
		gpio::Pin pin;
		unsigned char position;
		time::TimeType minPosition;
		time::TimeType maxPosition;
		std::thread* runThread;
		bool keepRunning;
	};
}