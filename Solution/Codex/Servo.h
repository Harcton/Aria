#pragma once
#include <thread>
#include <mutex>
#include <iostream>
#include <bcm2835.h>
#include "GPIO.h"
#include "Time.h"
#include "Assert.h"


namespace codex
{
	class Servo
	{
	public:
		Servo()
			: pin(gpio::pin_none)
			, position(0)
			, minPosition(0)
			, maxPosition(0)
			, runThread(nullptr)
			, keepRunning(false)
		{
			std::cout << "\nCreating a servo... 2";
		}
		~Servo()
		{
			stop();
		}

		void run()
		{
			if (isRunning())
				return;
			std::lock_guard<std::recursive_mutex> lock(mutex);

			//Set to keep running
			keepRunning = true;

			//Start a new thread that runs the servo
			runThread = new std::thread(&Servo::runLoop, this);
		}
		void stop()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			if (runThread)
			{
				keepRunning = false;
				runThread->join();
				delete runThread;
				runThread = nullptr;
			}
		}

		void setPosition(const unsigned char _position)
		{
			mutex.lock();
			position = _position;
			const float posPercentage = (float)position / 255.0f;
			const time::TimeType pulseDuration = minPosition + time::TimeType(float(maxPosition - minPosition) * posPercentage);
			std::cout << "\nPulse duration set to: " << std::to_string(pulseDuration);
			mutex.unlock();
		}
		void setPin(const gpio::Pin _pin)
		{
			mutex.lock();
			pin = _pin;
			mutex.unlock();
			bcm2835_gpio_fsel(_pin, BCM2835_GPIO_FSEL_OUTP);
		}
		void mapPosition(const time::TimeType minPulseWidth, const time::TimeType maxPulseWidth)
		{
			mutex.lock();
			minPosition = minPulseWidth;
			maxPosition = maxPulseWidth;
			//assert(minPulseWidth < maxPulseWidth);
			//file_line_assert(__FILE__, __LINE__, minPulseWidth < maxPulseWidth);
			mutex.unlock();
		}

		gpio::Pin getPin() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return pin;
		}
		unsigned char getPosition() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return position;
		}
		bool isRunning() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return keepRunning;
		}

		void runLoop()
		{
			mutex.lock();
			bool _keepRunning = keepRunning;
			mutex.unlock();

			while (_keepRunning)
			{
				std::cout << ".";
				//Enable
				mutex.lock();
				gpio::enable(pin);
				//Delay
				const float posPercentage = (float)position / 255.0f;
				const time::TimeType pulseDuration = minPosition + time::TimeType(float(maxPosition - minPosition) * posPercentage);
				time::delay(time::nanoseconds(pulseDuration));
				//Disable
				gpio::disable(pin);
				_keepRunning = keepRunning;
				mutex.unlock();
				//Delay before the next pulse
				time::delay(time::milliseconds(20));
			}
		}
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