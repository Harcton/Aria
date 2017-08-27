#pragma once
#include <stdint.h>
#undef delay //"delay" is a macro defined in the bcm header...


namespace codex
{
	namespace time
	{
		typedef uint64_t TimeType;
		namespace conversionRate
		{
			static const TimeType second = 1000000000;
			static const TimeType millisecond = 1000000;
			static const TimeType nanosecond = 1;
		}

		inline TimeType seconds(const float seconds)
		{
			return seconds * conversionRate::second;
		}
		inline TimeType milliseconds(const float milliseconds)
		{
			return milliseconds * conversionRate::millisecond;
		}
		inline TimeType nanoseconds(const uint64_t nanoseconds)
		{
			return nanoseconds * conversionRate::nanosecond;
		}

		TimeType getRunTime();

		/* Delay code execution by specified amount of time */
		void delay(const TimeType time);
	}
}