#include "Servo.h"


namespace codex
{

	Servo::Servo()
		: pin(gpio::pin_none)
		, position(0)
		, minPosition(0)
		, maxPosition(0)
		, runThread(nullptr)
		, keepRunning(false)
	{
		codex::log::info("Creating a servo...");
	}

	Servo::~Servo()
	{
		stop();
	}

	void Servo::run()
	{
		if (isRunning())
			return;
		std::lock_guard<std::recursive_mutex> lock(mutex);

		//Set to keep running
		keepRunning = true;

		//Start a new thread that runs the servo
		runThread = new std::thread(&Servo::runLoop, this);
	}

	void Servo::stop()
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

	void Servo::setPosition(const unsigned char _position)
	{
		mutex.lock();
		position = _position;
		const float posPercentage = (float)position / 255.0f;
		const time::TimeType pulseDuration = minPosition + time::TimeType(float(maxPosition - minPosition) * posPercentage);
		std::cout << "\nPulse duration set to: " << std::to_string(pulseDuration);
		mutex.unlock();
	}

	void Servo::setPin(const gpio::Pin _pin)
	{
		mutex.lock();
		pin = _pin;
		mutex.unlock();
		bcm2835_gpio_fsel(_pin, BCM2835_GPIO_FSEL_OUTP);
	}

	void Servo::mapPosition(const time::TimeType minPulseWidth, const time::TimeType maxPulseWidth)
	{
		mutex.lock();
		minPosition = minPulseWidth;
		maxPosition = maxPulseWidth;
		//assert(minPulseWidth < maxPulseWidth);
		//file_line_assert(__FILE__, __LINE__, minPulseWidth < maxPulseWidth);
		mutex.unlock();
	}

	gpio::Pin Servo::getPin() const
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		return pin;
	}

	unsigned char Servo::getPosition() const
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		return position;
	}

	bool Servo::isRunning() const
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		return keepRunning;
	}

	void Servo::runLoop()
	{
		mutex.lock();
		bool _keepRunning = keepRunning;
		mutex.unlock();

		while (_keepRunning)
		{
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
}