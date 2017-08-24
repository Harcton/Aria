#pragma once
#include <stdint.h>
#undef delay //"delay" is a macro defined in the bcm header...


namespace codex
{
	namespace time
	{
		typedef uint64_t TimeType;

		inline TimeType seconds(const float seconds)
		{
			return seconds * 1000000000.0f;
		}
		inline TimeType milliseconds(const float milliseconds)
		{
			return milliseconds * 1000000.0f;
		}
		inline TimeType nanoseconds(const uint64_t nanoseconds)
		{
			return nanoseconds;
		}

		/* Delay code execution by specified amount of time */
		void delay(const TimeType time);
	}
}