#include "Servo.h"
#include <assert.h>
#include <algorithm>

namespace codex
{
	namespace device
	{
		Servo::Servo()
			: targetAngle(0.0f)
			, approximatedAngle(0.0f)
			, lastUpdateTime(0)
			, pin(gpio::Pin::pin_none)
			, minPulseWidth(0)
			, maxPulseWidth(0)
			, rotationSpeed(0.0f)
			, minAngle(0.0f)
			, maxAngle(0.0f)
		{
		}

		Servo::~Servo()
		{
			stop();
			while (isRunning()) {/*Blocks*/ }
		}

		void Servo::setTargetAngle(const float target)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			targetAngle = target;
		}

		void Servo::setPin(const gpio::Pin _pin)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			pin = _pin;
			bcm2835_gpio_fsel(_pin, BCM2835_GPIO_FSEL_OUTP);
		}

		void Servo::setAngleLimits(const time::TimeType _minPulseWidth, const time::TimeType _maxPulseWidth, const float _minAngle, const float _maxAngle)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			minPulseWidth = _minPulseWidth;
			maxPulseWidth = _maxPulseWidth;
			minAngle = _minAngle;
			maxAngle = _maxAngle;
		}

		void Servo::setMinAngle(const time::TimeType _minPulseWidth, const float _minAngle)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			minPulseWidth = _minPulseWidth;
			minAngle = _minAngle;
		}

		void Servo::setMaxAngle(const time::TimeType _maxPulseWidth, const float _maxAngle)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			maxPulseWidth = _maxPulseWidth;
			maxAngle = _maxAngle;
		}

		void Servo::setRotationSpeed(const float speed)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			rotationSpeed = speed;
		}

		gpio::Pin Servo::getPin() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return pin;
		}

		float Servo::getApproximatedAngle() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return approximatedAngle;
		}

		float Servo::getRotationSpeed() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return rotationSpeed;
		}

		void Servo::onStart()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			lastUpdateTime = time::getRunTime();
		}

		void Servo::update()
		{
			const time::TimeType updateInterval = time::milliseconds(5);

			mutex.lock();
			if (pin == gpio::pin_none)
			{
				mutex.unlock();
				return;
			}

			//Enable
			gpio::enable(pin);
			//Delay
			const float target = std::min(maxAngle, std::max(targetAngle, minAngle));
			const float posPercentage = (target - minAngle) / (maxAngle - minAngle);
			const time::TimeType pulseDuration = minPulseWidth + time::TimeType(float(maxPulseWidth - minPulseWidth) * posPercentage);
			mutex.unlock();
			time::delay(pulseDuration);
			mutex.lock();
			//Disable
			gpio::disable(pin);

			//Angle approximation
			if (rotationSpeed <= 0.0f)
			{//No rotation speed provided, assume angle assumation to the target angle
				approximatedAngle = targetAngle;
			}
			else
			{//Approximate delta movement
				const time::TimeType spentTime = updateInterval + pulseDuration;
				if (approximatedAngle > targetAngle)
				{
					approximatedAngle -= rotationSpeed * time::toSeconds(spentTime);
					if (approximatedAngle < targetAngle)
						approximatedAngle = targetAngle;
				}
				else if (approximatedAngle < targetAngle)
				{
					approximatedAngle += rotationSpeed * time::toSeconds(spentTime);
					if (approximatedAngle > targetAngle)
						approximatedAngle = targetAngle;
				}
			}

			//Delay the next update
			mutex.unlock();
			time::delay(updateInterval);
		}
	}
}