#pragma once
#include <thread>
#include <mutex>
#include <iostream>
#include <bcm2835.h>
#include "GPIO.h"
#include "CodexTime.h"
#include "CodexAssert.h"
#include "ThreadedDevice.h"
#include "Sync/ISyncType.h"



namespace codex
{
	namespace device
	{
		class IServo : public sync::IType
		{
		public:
			
			//Initialization
			/* Sets the pin that the servo is connected to. */
			virtual void setPin(const gpio::Pin pin) = 0;
			/* Set the pulse durations used to set the servo to its minimum and maximum angles. */
			virtual void setAngleLimits(const time::TimeType minPulseWidth, const time::TimeType maxPulseWidth, const float minAngle, const float maxAngle) = 0;
			virtual void setMinAngle(const time::TimeType minPulseWidth, const float minAngle) = 0;
			virtual void setMaxAngle(const time::TimeType maxPulseWidth, const float maxAngle) = 0;
			/* Set a rotation speed per second approximation. */
			virtual void setRotationSpeed(const float speed) = 0;

			//Control
			virtual void setActive(const bool isActive) = 0;
			virtual void setTargetAngle(const float angle) = 0;

			//State query
			virtual gpio::Pin getPin() const = 0;
			virtual float getApproximatedAngle() const = 0;
			virtual float getRotationSpeed() const = 0;
			virtual float getMinAngle() const = 0;
			virtual float getMaxAngle() const = 0;
		};



		class ServoGhost final : public IServo
		{
			CODEX_SYNC_TYPE_DECL(ServoGhost, 0)
		public:
			ServoGhost(const std::string& n);
			~ServoGhost();
			
			//Servo interface
			void setPin(const gpio::Pin pin) override;
			void setAngleLimits(const time::TimeType minPulseWidth, const time::TimeType maxPulseWidth, const float minAngle, const float maxAngle) override;
			void setMinAngle(const time::TimeType minPulseWidth, const float minAngle) override;
			void setMaxAngle(const time::TimeType maxPulseWidth, const float maxAngle) override;
			void setRotationSpeed(const float speed) override;
			void setActive(const bool isActive) override;
			void setTargetAngle(const float angle) override;
			gpio::Pin getPin() const override;
			float getApproximatedAngle() const override;
			float getRotationSpeed() const override;
			float getMinAngle() const override;
			float getMaxAngle() const override;

			//Sync type
			void syncCreate(protocol::WriteBuffer& buffer) override;
			void syncCreate(protocol::ReadBuffer& buffer) override;
			void syncRemove(protocol::WriteBuffer& buffer) override;
			void syncRemove(protocol::ReadBuffer& buffer) override;
			bool syncUpdate(const time::TimeType& deltaTime) override;
			void syncUpdate(protocol::WriteBuffer& buffer) override;
			void syncUpdate(protocol::ReadBuffer& buffer) override;

			const std::string name;

		private:
			float targetAngle;
			float approximatedAngle;
			gpio::Pin pin;
			time::TimeType minPulseWidth;
			time::TimeType maxPulseWidth;
			float rotationSpeed;
			float minAngle;
			float maxAngle;
			bool active;

			bool syncRequested;
		};
		


		class ServoShell final : public ThreadedDevice, public IServo
		{
			CODEX_SYNC_TYPE_DECL(ServoShell, 0)
		public:
			ServoShell();
			~ServoShell();

			//Servo interface
			void setPin(const gpio::Pin pin) override;
			void setAngleLimits(const time::TimeType minPulseWidth, const time::TimeType maxPulseWidth, const float minAngle, const float maxAngle) override;
			void setMinAngle(const time::TimeType minPulseWidth, const float minAngle) override;
			void setMaxAngle(const time::TimeType maxPulseWidth, const float maxAngle) override;
			void setRotationSpeed(const float speed) override;
			void setActive(const bool isActive) override;
			void setTargetAngle(const float angle) override;
			gpio::Pin getPin() const override;
			float getApproximatedAngle() const override;
			float getRotationSpeed() const override;
			float getMinAngle() const override;
			float getMaxAngle() const override;
			
			//Sync type
			void syncCreate(protocol::WriteBuffer& buffer) override;
			void syncCreate(protocol::ReadBuffer& buffer) override;
			void syncRemove(protocol::WriteBuffer& buffer) override;
			void syncRemove(protocol::ReadBuffer& buffer) override;
			bool syncUpdate(const time::TimeType& deltaTime) override;
			void syncUpdate(protocol::WriteBuffer& buffer) override;
			void syncUpdate(protocol::ReadBuffer& buffer) override;

		private:
			//Threaded device
			void onStart() override;
			void update() override;
			void onStop() override;

			mutable std::recursive_mutex mutex;
			time::TimeType lastUpdateTime;
			float targetAngle;
			float approximatedAngle;
			gpio::Pin pin;
			time::TimeType minPulseWidth;
			time::TimeType maxPulseWidth;
			float rotationSpeed;
			float minAngle;
			float maxAngle;
			bool active;

			time::TimeType updateTimer;
		};
	}
}