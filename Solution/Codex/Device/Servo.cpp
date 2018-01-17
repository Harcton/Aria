#include "Servo.h"
#include "Protocol.h"
#include "SpehsEngine/Core/Log.h"
#include <assert.h>
#include <algorithm>

namespace codex
{
	namespace device
	{
		static const int debugLevel = 2;

		ServoGhost::ServoGhost()
			: name()
			, targetAngle(0.0f)
			, approximatedAngle(0.0f)
			, pin(gpio::Pin::pin_none)
			, minPulseWidth(0)
			, maxPulseWidth(0)
			, rotationSpeed(0.0f)
			, minAngle(0.0f)
			, maxAngle(0.0f)
			, active(false)
			, syncRequested(true)
		{
		}

		ServoGhost::~ServoGhost()
		{
		}

		void ServoGhost::setActive(const bool isActive)
		{
			active = isActive;
			syncRequested = true;
		}

		void ServoGhost::setTargetAngle(const float target)
		{
			targetAngle = target;
			syncRequested = true;
		}

		void ServoGhost::setPin(const gpio::Pin _pin)
		{
			pin = _pin;
		}

		void ServoGhost::setAngleLimits(const spehs::time::Time _minPulseWidth, const spehs::time::Time _maxPulseWidth, const float _minAngle, const float _maxAngle)
		{
			minPulseWidth = _minPulseWidth;
			maxPulseWidth = _maxPulseWidth;
			minAngle = _minAngle;
			maxAngle = _maxAngle;
		}

		void ServoGhost::setMinAngle(const spehs::time::Time _minPulseWidth, const float _minAngle)
		{
			minPulseWidth = _minPulseWidth;
			minAngle = _minAngle;
		}

		void ServoGhost::setMaxAngle(const spehs::time::Time _maxPulseWidth, const float _maxAngle)
		{
			maxPulseWidth = _maxPulseWidth;
			maxAngle = _maxAngle;
		}

		void ServoGhost::setRotationSpeed(const float speed)
		{
			rotationSpeed = speed;
		}

		gpio::Pin ServoGhost::getPin() const
		{
			return pin;
		}

		float ServoGhost::getApproximatedAngle() const
		{
			return approximatedAngle;
		}

		float ServoGhost::getRotationSpeed() const
		{
			return rotationSpeed;
		}
		
		float ServoGhost::getMinAngle() const
		{
			return minAngle;
		}

		float ServoGhost::getMaxAngle() const
		{
			return maxAngle;
		}

		void ServoGhost::syncCreate(protocol::WriteBuffer& buffer)
		{
			buffer.write(pin);
			buffer.write(minPulseWidth);
			buffer.write(maxPulseWidth);
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

		bool ServoGhost::syncUpdate(const spehs::time::Time deltaTime)
		{
			return syncRequested;
		}

		void ServoGhost::syncUpdate(protocol::WriteBuffer& buffer)
		{
			buffer.write(active);
			buffer.write(targetAngle);
		}

		void ServoGhost::syncUpdate(protocol::ReadBuffer& buffer)
		{
			buffer.read(approximatedAngle);
		}

		

		//SHELL
		ServoShell::ServoShell()
			: lastUpdateTime(0)
			, targetAngle(0.0f)
			, approximatedAngle(0.0f)
			, pin(gpio::Pin::pin_none)
			, minPulseWidth(0)
			, maxPulseWidth(0)
			, rotationSpeed(0.0f)
			, minAngle(0.0f)
			, maxAngle(0.0f)
			, active(false)
		{
			spehs::log::info("ServoShell constructor.");
		}

		ServoShell::~ServoShell()
		{
			stop();
			while (isRunning()) {/*Blocks*/ }
			std::lock_guard<std::recursive_mutex> lock(mutex);
			spehs::log::info("ServoShell destructor.");
		}

		void ServoShell::setActive(const bool isActive)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			if (active != isActive)
			{
				active = isActive;
				if (active)
				{
					start();
				}
				else
				{
					stop();
				}
			}
		}

		void ServoShell::setTargetAngle(const float target)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			targetAngle = target;
		}

		void ServoShell::setPin(const gpio::Pin _pin)
		{
			gpio::setPinAsOutput(_pin);
			std::lock_guard<std::recursive_mutex> lock(mutex);
			pin = _pin;
		}

		void ServoShell::setAngleLimits(const spehs::time::Time _minPulseWidth, const spehs::time::Time _maxPulseWidth, const float _minAngle, const float _maxAngle)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			minPulseWidth = _minPulseWidth;
			maxPulseWidth = _maxPulseWidth;
			minAngle = _minAngle;
			maxAngle = _maxAngle;
		}

		void ServoShell::setMinAngle(const spehs::time::Time _minPulseWidth, const float _minAngle)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			minPulseWidth = _minPulseWidth;
			minAngle = _minAngle;
		}

		void ServoShell::setMaxAngle(const spehs::time::Time _maxPulseWidth, const float _maxAngle)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			maxPulseWidth = _maxPulseWidth;
			maxAngle = _maxAngle;
		}

		void ServoShell::setRotationSpeed(const float speed)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			rotationSpeed = speed;
		}

		gpio::Pin ServoShell::getPin() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return pin;
		}

		float ServoShell::getApproximatedAngle() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return approximatedAngle;
		}

		float ServoShell::getRotationSpeed() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return rotationSpeed;
		}

		float ServoShell::getMinAngle() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return minAngle;
		}

		float ServoShell::getMaxAngle() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return maxAngle;
		}

		void ServoShell::onStart()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			lastUpdateTime = spehs::time::now();
			if (debugLevel >= 1)
				spehs::log::info("ServoGhost::onStart: starting...");
		}

		void ServoShell::update()
		{
			const spehs::time::Time updateInterval = spehs::time::fromMilliseconds(5);

			mutex.lock();
			if (debugLevel >= 4)
				spehs::log::info("ServoGhost::update: pin: " + std::to_string(gpio::getPinEnumAsNumber(pin))
					+ ", speed: " + std::to_string(rotationSpeed)
					+ ", target: " + std::to_string(targetAngle)
					+ ", min Q: " + std::to_string(minPulseWidth.asMicroseconds())
					+ ", max Q: " + std::to_string(maxPulseWidth.asMicroseconds()));
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
			const spehs::time::Time pulseDuration = minPulseWidth + spehs::time::Time(float(maxPulseWidth - minPulseWidth) * posPercentage);
			mutex.unlock();
			spehs::time::delay(pulseDuration);
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
				const spehs::time::Time spentTime = updateInterval + pulseDuration;
				if (approximatedAngle > targetAngle)
				{
					approximatedAngle -= rotationSpeed * spentTime.asSeconds();
					if (approximatedAngle < targetAngle)
						approximatedAngle = targetAngle;
				}
				else if (approximatedAngle < targetAngle)
				{
					approximatedAngle += rotationSpeed * spentTime.asSeconds();
					if (approximatedAngle > targetAngle)
						approximatedAngle = targetAngle;
				}
			}

			//Delay the next update
			mutex.unlock();
			spehs::time::delay(updateInterval);
		}

		void ServoShell::onStop()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			lastUpdateTime = spehs::time::now();
			if (debugLevel >= 1)
				spehs::log::info("ServoGhost::onStop: stopping...");
		}

		void ServoShell::syncCreate(protocol::WriteBuffer& buffer)
		{

		}

		void ServoShell::syncCreate(protocol::ReadBuffer& buffer)
		{
			codex::gpio::Pin _pin;
			spehs::time::Time _minPulseWidth;
			spehs::time::Time _maxPulseWidth;
			float _rotationSpeed;
			float _minAngle;
			float _maxAngle;

			//Read specs
			buffer.read(_pin);
			buffer.read(_minPulseWidth);
			buffer.read(_maxPulseWidth);
			buffer.read(_rotationSpeed);
			buffer.read(_minAngle);
			buffer.read(_maxAngle);

			//Initialize
			setPin(_pin);
			setAngleLimits(_minPulseWidth, _maxPulseWidth, _minAngle, _maxAngle);
			setRotationSpeed(_rotationSpeed);

			spehs::log::info("ServoShell initialized. Pin: " + std::to_string(gpio::getPinEnumAsNumber(_pin))
				+ ", min pulse width: " + std::to_string(_minPulseWidth.asMicroseconds())
				+ ", max pulse width: " + std::to_string(_minPulseWidth.asMicroseconds())
				+ ", min angle: " + std::to_string(_minAngle)
				+ ", max angle: " + std::to_string(_maxAngle)
				+ ", rotation speed: " + std::to_string(_rotationSpeed));
		}

		void ServoShell::syncRemove(protocol::WriteBuffer& buffer)
		{

		}

		void ServoShell::syncRemove(protocol::ReadBuffer& buffer)
		{
			stop();
			while(isRunning()) {}
		}

		bool ServoShell::syncUpdate(const spehs::time::Time deltaTime)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			updateTimer -= deltaTime;
			if (updateTimer <= spehs::time::zero)
			{
				updateTimer = spehs::time::fromSeconds(1.0 / 30.0f);
				return true;
			}
			else
			{
				return false;
			}
		}

		void ServoShell::syncUpdate(protocol::WriteBuffer& buffer)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			buffer.write(approximatedAngle);
		}

		void ServoShell::syncUpdate(protocol::ReadBuffer& buffer)
		{
			bool _active;
			float _targetAngle;
			buffer.read(_active);
			buffer.read(_targetAngle);
			setActive(_active);
			setTargetAngle(_targetAngle);
		}
	}
}