#pragma once
#include <thread>
#include <mutex>
#include <iostream>
#include <bcm2835.h>
#include "GPIO.h"
#include "CodexTime.h"
#include "CodexAssert.h"
#include "ThreadedDevice.h"


namespace codex
{
	/*
		Note that the angles used with the servo class can be of any unit.
		The only requirement is that max angles are always greater than min angles.
	*/
	class Servo : public ThreadedDevice
	{
	public:
		Servo();
		~Servo();

		//Control
		void setTargetAngle(const float angle);

		//Initialization
		/* Sets the pin that the servo is connected to. */
		void setPin(const gpio::Pin pin);
		/* Set the pulse durations used to set the servo to its minimum and maximum angles. */
		void setAngleLimits(const time::TimeType minPulseWidth, const time::TimeType maxPulseWidth, const float minAngle, const float maxAngle);
		void setMinAngle(const time::TimeType minPulseWidth, const float minAngle);
		void setMaxAngle(const time::TimeType maxPulseWidth, const float maxAngle);
		/* Set a rotation speed per second approximation. */
		void setRotationSpeed(const float speed);

		gpio::Pin getPin() const;
		float getApproximatedAngle() const;
		float getRotationSpeed() const;

	private:
		void onStart() override;
		void update() override;

		mutable std::recursive_mutex mutex;
		float targetAngle;
		float approximatedAngle;
		time::TimeType lastUpdateTime;

		//Hardware bound specifications
		gpio::Pin pin;
		time::TimeType minPulseWidth;
		time::TimeType maxPulseWidth;
		float rotationSpeed;
		float minAngle;
		float maxAngle;
	};
}