#pragma once
#include <stdint.h>
#include <chrono>
#undef delay //"delay" is a macro defined in the bcm header...


namespace codex
{
	namespace time
	{
		typedef int64_t TimeType;
		namespace conversionRate
		{
			static const TimeType second = std::chrono::high_resolution_clock::time_point::period::den;
			static const TimeType millisecond = second / 1000;
			static const TimeType microsecond = millisecond / 1000;
			static const TimeType nanosecond = microsecond / 1000;
		}

		inline TimeType seconds(const float seconds)
		{
			return seconds * conversionRate::second;
		}
		inline TimeType milliseconds(const float milliseconds)
		{
			return milliseconds * conversionRate::millisecond;
		}
		inline TimeType microseconds(const float microseconds)
		{
			return microseconds * conversionRate::microsecond;
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
		inline float toMicroseconds(const TimeType time)
		{
			return (float)time / (float)conversionRate::microsecond;
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