#include "DCMotorController.h"
#include "SpehsEngine/Core/Time.h"
#include "SpehsEngine/Core/Log.h"
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
			, pulseInterval(spehs::time::fromMilliseconds(2.0f))
		{

		}

		DCMotorController::~DCMotorController()
		{
			stop();
			while (isRunning()) {/*Blocks*/ }
		}

		void DCMotorController::setPins(const gpio::Pin _pulseWidthPin, const gpio::Pin _inputPin1, gpio::Pin _inputPin2)
		{
			gpio::setPinAsOutput(_pulseWidthPin);
			gpio::setPinAsOutput(_inputPin1);
			gpio::setPinAsOutput(_inputPin2);
			std::lock_guard<std::recursive_mutex> lock(mutex);
			pulseWidthPin = _pulseWidthPin;
			inputPin1 = _inputPin1;
			inputPin2 = _inputPin2;
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
			pulseWidth = pulseInterval * abs(strength);

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
			const spehs::time::Time _pulseWidth = pulseWidth;
			const spehs::time::Time _pulseInterval = pulseInterval;
			mutex.unlock();

			codex::gpio::enable(_pulseWidthPin);
			spehs::time::delay(_pulseWidth);
			codex::gpio::disable(_pulseWidthPin);
			spehs::time::delay(_pulseInterval - _pulseWidth);
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

const spehs::time::Time pulseInterval = spehs::time::fromNanoseconds(2000000);
int direction = 1;
spehs::time::Time data = 0;

while (keepRunning)
{
std::lock_guard<std::recursive_mutex> lock(mutex);

if (data <= 0)
direction = 1000;
else if (data >= pulseInterval)
direction = -1000;
data += direction;
codex::gpio::enable(pulseWidthPin);
spehs::time::delay(data);
codex::gpio::disable(pulseWidthPin);
spehs::time::delay(pulseInterval - data);

keepRunning = !stopRequested;
}

*/