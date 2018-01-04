#include "Codex/Device/HC_SR04.h"
#include "SpehsEngine/Core/Time.h"
#include "SpehsEngine/Core/Log.h"


namespace codex
{
	namespace device
	{
		HC_SR04::HC_SR04()
			: triggerPin(gpio::pin_none)
			, echoPin(gpio::pin_none)
			, distance(0.0f)
			, pollInterval(spehs::time::fromMilliseconds(100))
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
			const spehs::time::Time delay1 = spehs::time::fromMilliseconds(2);
			const spehs::time::Time delay2 = spehs::time::fromMilliseconds(10);
			const spehs::time::Time beginTime = spehs::time::now();
			gpio::disable(trigger);
			spehs::time::delay(delay1);
			gpio::enable(trigger);
			spehs::time::delay(delay2);
			gpio::disable(trigger);

			//Measure pulse
			const spehs::time::Time duration = gpio::pulseIn(echo, gpio::high, spehs::time::fromMilliseconds(1000));

			//Update distance value
			mutex.lock();
			distance = (duration.asMicroseconds() * 0.5f) / 29.1;
			//Delay by poll interval
			const spehs::time::Time interval = pollInterval + beginTime - spehs::time::now();
			mutex.unlock();
			spehs::time::delay(interval);
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