#include "CodexTime.h"

namespace codex
{
	namespace time
	{
		static std::chrono::high_resolution_clock::rep beginRunTime = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		
		TimeType getRunTime()
		{
			const std::chrono::high_resolution_clock::rep now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
			return now - beginRunTime;
		}

		void delay(TimeType time)
		{
			const std::chrono::high_resolution_clock::rep begin = std::chrono::high_resolution_clock::now().time_since_epoch().count();
			while (1)
			{
				const std::chrono::high_resolution_clock::rep now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
				if (now - begin >= time)
					break;
			}
		}
	}
}
