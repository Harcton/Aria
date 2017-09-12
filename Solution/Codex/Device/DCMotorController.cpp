#include "DCMotorController.h"
#include "CodexTime.h"
#include "Log.h"
#include <cmath>
#include <bcm2835.h>


namespace codex
{
	namespace device
	{
		DCMotorController::DCMotorController()
			: pulseWidthPin(gpio::Pin::pin_none)
			, inputPin1(gpio::Pin::pin_none)
			, inputPin2(gpio::Pin::pin_none)
			, pulseWidth(0)
			, strength(0.0f)
			, pulseInterval(codex::time::milliseconds(2.0f))
		{

		}

		DCMotorController::~DCMotorController()
		{
			stop();
			while (isRunning()) {/*Blocks*/ }
		}

		void DCMotorController::setPins(const gpio::Pin _pulseWidthPin, const gpio::Pin _inputPin1, gpio::Pin _inputPin2)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			pulseWidthPin = _pulseWidthPin;
			inputPin1 = _inputPin1;
			inputPin2 = _inputPin2;
			bcm2835_gpio_fsel(_pulseWidthPin, BCM2835_GPIO_FSEL_OUTP);
			bcm2835_gpio_fsel(_inputPin1, BCM2835_GPIO_FSEL_OUTP);
			bcm2835_gpio_fsel(_inputPin2, BCM2835_GPIO_FSEL_OUTP);
		}

		void DCMotorController::setStrength(const float newStrength)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);

			//Cap strength in range
			strength = newStrength;
			if (strength < -1.0f)
				strength = -1.0f;
			else if (strength > 1.0f)
				strength = 1.0f;

			//Calculate pulse width
			pulseWidth = abs(strength) * (float)pulseInterval;

			//Polarity
			if (strength > 0.0f)
			{
				codex::gpio::disable(inputPin1);
				codex::gpio::enable(inputPin2);
			}
			else if (strength < 0.0f)
			{
				codex::gpio::enable(inputPin1);
				codex::gpio::disable(inputPin2);
			}
			else
			{
				codex::gpio::disable(inputPin1);
				codex::gpio::disable(inputPin2);
			}
		}

		float DCMotorController::getStrength() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return strength;
		}

		void DCMotorController::onStart()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			setStrength(getStrength());
		}

		void DCMotorController::update()
		{
			mutex.lock();
			const codex::gpio::Pin _pulseWidthPin = pulseWidthPin;
			const codex::time::TimeType _pulseWidth = pulseWidth;
			const codex::time::TimeType _pulseInterval = pulseInterval;
			mutex.unlock();

			codex::gpio::enable(_pulseWidthPin);
			codex::time::delay(_pulseWidth);
			codex::gpio::disable(_pulseWidthPin);
			codex::time::delay(_pulseInterval - _pulseWidth);
		}

		void DCMotorController::onStop()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			codex::gpio::disable(inputPin1);
			codex::gpio::disable(inputPin2);
		}
	}
}


/*

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

*/