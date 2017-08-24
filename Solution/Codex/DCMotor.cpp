#include "DCMotor.h"
#include "CodexTime.h"
#include <bcm2835.h>


namespace codex
{
	DCMotor::DCMotor()
		: runThread(nullptr)
		, stopRequested(false)
		, pulseWidthPin(gpio::Pin::pin_none)
		, inputPin1(gpio::Pin::pin_none)
		, inputPin2(gpio::Pin::pin_none)
	{

	}
	
	DCMotor::~DCMotor()
	{
		stop();
		while (!mutex.try_lock())
		{
			//Blocks until mutex has been claimed
		}
	}

	void DCMotor::run()
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (runThread)
			return;
		stopRequested = false;
		runThread = new std::thread(&DCMotor::runLoop, this);
	}

	void DCMotor::stop()
	{
		mutex.lock();
		stopRequested = true;

		if (runThread)
		{
			mutex.unlock();
			runThread->join();
			mutex.lock();
			delete runThread;
			runThread = nullptr;
		}
		mutex.unlock();
	}

	void DCMotor::runLoop()
	{
		mutex.lock();
		codex::gpio::disable(inputPin1);
		codex::gpio::enable(inputPin2);
		mutex.unlock();

		bool keepRunning = true;
		const codex::time::TimeType pulseInterval = codex::time::nanoseconds(2000000);
		int direction = 1;
		codex::time::TimeType data = 0;

		while (keepRunning)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);

			if (data <= 0)
				direction = 1000;
			else if (data >= pulseInterval)
				direction = -1000;
			data += direction;
			codex::gpio::enable(pulseWidthPin);
			codex::time::delay(data);
			codex::gpio::disable(pulseWidthPin);
			codex::time::delay(pulseInterval - data);

			keepRunning = !stopRequested;
		}

		//Stop
		mutex.lock();
		codex::gpio::disable(inputPin1);
		codex::gpio::disable(inputPin2);
		mutex.unlock();
	}

	void DCMotor::setPins(const gpio::Pin _pulseWidthPin, const gpio::Pin _inputPin1, gpio::Pin _inputPin2)
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (runThread)
		{
			//TODO...
			return;
		}

		pulseWidthPin = _pulseWidthPin;
		inputPin1 = _inputPin1;
		inputPin2 = _inputPin2;

		bcm2835_gpio_fsel(_pulseWidthPin, BCM2835_GPIO_FSEL_OUTP);
		bcm2835_gpio_fsel(_inputPin1, BCM2835_GPIO_FSEL_OUTP);
		bcm2835_gpio_fsel(_inputPin2, BCM2835_GPIO_FSEL_OUTP);
	}

}