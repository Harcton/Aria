#pragma once
#include "Codex/Device/ThreadedDevice.h"
#include "Codex/GPIO.h"

namespace codex
{
	namespace device
	{
		class PIR_MotionSensor : public ThreadedDevice
		{
		public:
			PIR_MotionSensor();
			~PIR_MotionSensor();

			void setPin(const gpio::Pin pin);
			bool getMovement() const;

			void onStart() override;
			void update() override;
			void onStop() override;

		private:
			mutable std::recursive_mutex mutex;
			gpio::Pin pin;
			gpio::PinState previousPinState;
		};
	}
}