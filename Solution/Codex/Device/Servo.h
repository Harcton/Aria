#pragma once
#include <thread>
#include <mutex>
#include <iostream>
#include <bcm2835.h>
#include "GPIO.h"
#include "CodexTime.h"
#include "CodexAssert.h"
#include "ThreadedDevice.h"
#include "SyncType.h"



namespace codex
{
	namespace device
	{
		class AbstractServo : public ISyncType
		{
		public:

			AbstractServo();
			~AbstractServo();

			//Initialization
			/* Sets the pin that the servo is connected to. */
			virtual void setPin(const gpio::Pin pin);
			/* Set the pulse durations used to set the servo to its minimum and maximum angles. */
			virtual void setAngleLimits(const time::TimeType minPulseWidth, const time::TimeType maxPulseWidth, const float minAngle, const float maxAngle);
			virtual void setMinAngle(const time::TimeType minPulseWidth, const float minAngle);
			virtual void setMaxAngle(const time::TimeType maxPulseWidth, const float maxAngle);
			/* Set a rotation speed per second approximation. */
			virtual void setRotationSpeed(const float speed);

			//Control
			virtual void setTargetAngle(const float angle);

			//State query
			virtual gpio::Pin getPin() const;
			virtual float getApproximatedAngle() const;
			virtual float getRotationSpeed() const;

		protected:
			mutable std::recursive_mutex mutex;
			float targetAngle;
			float approximatedAngle;
			gpio::Pin pin;
			time::TimeType minPulseWidth;
			time::TimeType maxPulseWidth;
			float rotationSpeed;
			float minAngle;
			float maxAngle;
		};



		class ServoGhost : public AbstractServo
		{
			CODEX_SYNCTYPE_DECL(ServoGhost, 0)
		public:
			ServoGhost();
			~ServoGhost();

			//Sync type
			void syncCreate(protocol::WriteBuffer& buffer) override;
			void syncCreate(protocol::ReadBuffer& buffer) override;
			void syncRemove(protocol::WriteBuffer& buffer) override;
			void syncRemove(protocol::ReadBuffer& buffer) override;
			bool syncUpdate(const time::TimeType& deltaTime) override;
			void syncUpdate(protocol::WriteBuffer& buffer) override;
			void syncUpdate(protocol::ReadBuffer& buffer) override;

		private:

		};
		


		class ServoShell : public ThreadedDevice, public AbstractServo
		{
			CODEX_SYNCTYPE_DECL(ServoShell, 0)
		public:
			ServoShell();
			~ServoShell();
			
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

			time::TimeType lastUpdateTime;
		};
	}
}