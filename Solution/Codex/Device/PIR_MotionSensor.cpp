#include "Codex/Device/PIR_MotionSensor.h"
#include "SpehsEngine/Core/Log.h"

namespace codex
{
	namespace device
	{
		PIR_MotionSensor::PIR_MotionSensor()
		{

		}

		PIR_MotionSensor::~PIR_MotionSensor()
		{
			stop();
		}
		
		void PIR_MotionSensor::setPin(const gpio::Pin _pin)
		{
			gpio::setPinAsInput(_pin);
			std::lock_guard<std::recursive_mutex> lock(mutex);
			pin = _pin;
		}

		bool PIR_MotionSensor::getMovement() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return previousPinState == gpio::PinState::high;
		}

		void PIR_MotionSensor::onStart()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);

			//Make the first read to detect a change
			if (gpio::read(pin) == gpio::PinState::high)
				previousPinState = gpio::PinState::low;
			else
				previousPinState = gpio::PinState::high;
		}

		void PIR_MotionSensor::update()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			const gpio::PinState readState = gpio::read(pin);

			if (readState != previousPinState)
			{//On state change
				previousPinState = readState;
			}
		}

		void PIR_MotionSensor::onStop()
		{

		}
	}
}