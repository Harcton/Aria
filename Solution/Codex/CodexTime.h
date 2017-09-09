#pragma once
#include <stdint.h>
#undef delay //"delay" is a macro defined in the bcm header...


namespace codex
{
	namespace time
	{
		typedef int64_t TimeType;
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
		inline float toSeconds(const TimeType time)
		{
			return (float)time / (float)conversionRate::second;
		}
		inline float toMilliseconds(const TimeType time)
		{
			return (float)time / (float)conversionRate::millisecond;
		}
		inline float toNanoseconds(const TimeType time)
		{
			return (float)time / (float)conversionRate::nanosecond;
		}
		
		TimeType getRunTime();

		/* Delay code execution by specified amount of time */
		void delay(const TimeType time);
	}
}