#pragma once
#include "Codex/Device/ThreadedDevice.h"
#include "Codex/GPIO.h"

namespace codex
{
	namespace device
	{
		/*
			Ultrasonic Distance Sensor Module
		*/
		class HC_SR04 : ThreadedDevice
		{
		public:
			HC_SR04();
			~HC_SR04() override;

			void onStart() override;
			void update() override;
			void onStop() override;

			void setPins(const gpio::Pin trigger, const gpio::Pin echo);
		
		private:
			std::recursive_mutex mutex;
			gpio::Pin triggerPin;
			gpio::Pin echoPin;
		};
	}
}