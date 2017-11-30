#include "Servo.h"
#include "Protocol.h"
#include <assert.h>
#include <algorithm>

namespace codex
{
	namespace device
	{
		AbstractServo::AbstractServo()
			: targetAngle(0.0f)
			, approximatedAngle(0.0f)
			, pin(gpio::Pin::pin_none)
			, minPulseWidth(0)
			, maxPulseWidth(0)
			, rotationSpeed(0.0f)
			, minAngle(0.0f)
			, maxAngle(0.0f)
		{
		}

		AbstractServo::~AbstractServo()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
		}

		void AbstractServo::setTargetAngle(const float target)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			targetAngle = target;
		}

		void AbstractServo::setPin(const gpio::Pin _pin)
		{
			gpio::setPinAsOutput(_pin);
			std::lock_guard<std::recursive_mutex> lock(mutex);
			pin = _pin;
		}

		void AbstractServo::setAngleLimits(const time::TimeType _minPulseWidth, const time::TimeType _maxPulseWidth, const float _minAngle, const float _maxAngle)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			minPulseWidth = _minPulseWidth;
			maxPulseWidth = _maxPulseWidth;
			minAngle = _minAngle;
			maxAngle = _maxAngle;
		}

		void AbstractServo::setMinAngle(const time::TimeType _minPulseWidth, const float _minAngle)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			minPulseWidth = _minPulseWidth;
			minAngle = _minAngle;
		}

		void AbstractServo::setMaxAngle(const time::TimeType _maxPulseWidth, const float _maxAngle)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			maxPulseWidth = _maxPulseWidth;
			maxAngle = _maxAngle;
		}

		void AbstractServo::setRotationSpeed(const float speed)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			rotationSpeed = speed;
		}

		gpio::Pin AbstractServo::getPin() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return pin;
		}

		float AbstractServo::getApproximatedAngle() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return approximatedAngle;
		}

		float AbstractServo::getRotationSpeed() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return rotationSpeed;
		}


		ServoGhost::ServoGhost()
		{
		}

		ServoGhost::~ServoGhost()
		{
		}
		
		void ServoGhost::syncCreate(protocol::WriteBuffer& buffer)
		{
			buffer.write<codex::gpio::Pin>(pin);
			buffer.write<codex::time::TimeType>(minPulseWidth);
			buffer.write<codex::time::TimeType>(maxPulseWidth);
			buffer.write(rotationSpeed);
			buffer.write(minAngle);
			buffer.write(maxAngle);
		}

		void ServoGhost::syncCreate(protocol::ReadBuffer& buffer)
		{

		}

		void ServoGhost::syncRemove(protocol::WriteBuffer& buffer)
		{

		}

		void ServoGhost::syncRemove(protocol::ReadBuffer& buffer)
		{

		}

		bool ServoGhost::syncUpdate(const time::TimeType& deltaTime)
		{
			return false;
		}

		void ServoGhost::syncUpdate(protocol::WriteBuffer& buffer)
		{

		}

		void ServoGhost::syncUpdate(protocol::ReadBuffer& buffer)
		{

		}




		//SHELL
		ServoShell::ServoShell()
			: lastUpdateTime(0)
		{
		}

		ServoShell::~ServoShell()
		{
			stop();
			while (isRunning()) {/*Blocks*/ }
		}

		void ServoShell::onStart()
		{
			std::lock_guard<std::recursive_mutex> lock(AbstractServo::mutex);
			lastUpdateTime = time::now();
		}

		void ServoShell::update()
		{
			const time::TimeType updateInterval = time::milliseconds(5);

			AbstractServo::mutex.lock();
			if (pin == gpio::pin_none)
			{
				AbstractServo::mutex.unlock();
				return;
			}

			//Enable
			gpio::enable(pin);
			//Delay
			const float target = std::min(maxAngle, std::max(targetAngle, minAngle));
			const float posPercentage = (target - minAngle) / (maxAngle - minAngle);
			const time::TimeType pulseDuration = minPulseWidth + time::TimeType(float(maxPulseWidth - minPulseWidth) * posPercentage);
			AbstractServo::mutex.unlock();
			time::delay(pulseDuration);
			AbstractServo::mutex.lock();
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
			AbstractServo::mutex.unlock();
			time::delay(updateInterval);
		}

		void ServoShell::syncCreate(protocol::WriteBuffer& buffer)
		{

		}

		void ServoShell::syncCreate(protocol::ReadBuffer& buffer)
		{
			codex::gpio::Pin _pin;
			codex::time::TimeType _minPulseWidth;
			codex::time::TimeType _maxPulseWidth;
			float _rotationSpeed;
			float _minAngle;
			float _maxAngle;

			//Read specs
			buffer.read<codex::gpio::Pin>(_pin);
			buffer.read<codex::time::TimeType>(_minPulseWidth);
			buffer.read<codex::time::TimeType>(_maxPulseWidth);
			buffer.read(_rotationSpeed);
			buffer.read(_minAngle);
			buffer.read(_maxAngle);

			//Initialize
			setPin(pin);
			setAngleLimits(_minPulseWidth, _maxPulseWidth, _minAngle, _maxAngle);
			setRotationSpeed(_rotationSpeed);
		}

		void ServoShell::syncRemove(protocol::WriteBuffer& buffer)
		{

		}

		void ServoShell::syncRemove(protocol::ReadBuffer& buffer)
		{

		}

		bool ServoShell::syncUpdate(const time::TimeType& deltaTime)
		{
			return false;
		}

		void ServoShell::syncUpdate(protocol::WriteBuffer& buffer)
		{

		}

		void ServoShell::syncUpdate(protocol::ReadBuffer& buffer)
		{

		}
	}
}