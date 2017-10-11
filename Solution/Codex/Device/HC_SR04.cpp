#include "Codex/Device/HC_SR04.h"
#include "Codex/CodexTime.h"
#include "Codex/Log.h"


namespace codex
{
	namespace device
	{
		HC_SR04::HC_SR04()
			: triggerPin(gpio::pin_none)
			, echoPin(gpio::pin_none)
			, distance(0.0f)
			, pollInterval(codex::time::milliseconds(100))
		{
		}

		HC_SR04::~HC_SR04()
		{
			stop();
		}

		void HC_SR04::setPins(const gpio::Pin trigger, const gpio::Pin echo)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			triggerPin = trigger;
			echoPin = echo;
			gpio::setPinAsOutput(trigger);
			gpio::setPinAsInput(echo);
		}

		void HC_SR04::onStart()
		{

		}

		void HC_SR04::update()
		{
			//Lock mutec for a short while when acquiring variable values
			mutex.lock();
			const gpio::Pin trigger = triggerPin;
			const gpio::Pin echo = echoPin;
			mutex.unlock();
			
			//Fire trigger
			const time::TimeType delay1 = time::milliseconds(2);
			const time::TimeType delay2 = time::milliseconds(10);
			const time::TimeType beginTime = time::now();
			gpio::disable(trigger);
			time::delay(delay1);
			gpio::enable(trigger);
			time::delay(delay2);
			gpio::disable(trigger);

			//Measure pulse
			const time::TimeType duration = gpio::pulseIn(echo, gpio::high, time::milliseconds(1000));

			//Update distance value
			mutex.lock();
			distance = (time::toMicroseconds(duration) * 0.5f) / 29.1;
			//Delay by poll interval
			const time::TimeType interval = pollInterval + beginTime - time::now();
			mutex.unlock();
			time::delay(interval);
		}

		void HC_SR04::onStop()
		{

		}

		float HC_SR04::getDistance() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return distance;
		}
	}
}