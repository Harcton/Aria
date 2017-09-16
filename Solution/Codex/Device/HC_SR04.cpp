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
			/*
			HC-SR04 Ping distance sensor]
			VCC to arduino 5v GND to arduino GND
			Echo to Arduino pin 13 Trig to Arduino pin 12
			Red POS to Arduino pin 11
			Green POS to Arduino pin 10
			560 ohm resistor to both LED NEG and GRD power rail
			More info at: http://goo.gl/kJ8Gl
			Original code improvements to the Ping sketch sourced from Trollmaker.com
			Some code and wiring inspired by http://en.wikiversity.org/wiki/User:Dstaub/robotcar
			*/
			
		}

		void HC_SR04::update()
		{
			//Lock mutec for a short while when acquiring variable values
			mutex.lock();
			const gpio::Pin trigger = triggerPin;
			const gpio::Pin echo = echoPin;
			mutex.unlock();
			
			//Fire trigger
			gpio::disable(trigger);
			time::delay(time::milliseconds(2));
			gpio::enable(trigger);
			time::delay(time::milliseconds(10));
			gpio::disable(trigger);

			const time::TimeType duration = gpio::pulseIn(echo, gpio::high, time::milliseconds(500));
			const float distance = (time::toMicroseconds(duration) * 0.5f) / 29.1;

			codex::log::info("HC_SR04 d: " + std::to_string(distance));

			time::delay(time::milliseconds(500));
		}

		void HC_SR04::onStop()
		{

		}
	}
}