#pragma once
#include <thread>
#include <mutex>
#include "GPIO.h"
#include "ThreadedDevice.h"


namespace codex
{
	class DCMotorController : public ThreadedDevice
	{
	public:
		DCMotorController();
		~DCMotorController();

		//Initialization
		/* Sets the pins that are used to control the DCMotorController. */
		void setPins(const gpio::Pin pulseWidthPin, const gpio::Pin inputPin1, gpio::Pin inputPin2);

		//Control
		/* Set strength within the range of [0.0f, 1.0f] */
		void setStrength(const float strength);

	private:
		void onStart() override;
		void update() override;
		void onStop() override;

		mutable std::recursive_mutex mutex;
		gpio::Pin pulseWidthPin;
		gpio::Pin inputPin1;
		gpio::Pin inputPin2;
		time::TimeType pulseWidth;
		const codex::time::TimeType pulseInterval;
	};
}