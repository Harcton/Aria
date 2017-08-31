#include <assert.h>
#include <ctime>
#include <boost/date_time.hpp>
#include "CodexTime.h"
#include "Log.h"
#ifdef PLATFORM_LINUX
//Linux includes
#include <time.h>
#endif
#ifdef _WIN32
//Windows includes
#include <windows.h>
#endif

namespace codex
{
	namespace time
	{
		TimeType getRunTime()
		{
			return clock() * conversionRate::second / CLOCKS_PER_SEC;
			//return GetTickCount();//Windows
		}

		void delay(TimeType time)
		{
#ifdef _WIN32
			assert(time >= 1000000);
			//On windows we don't have - and most likely don't need - a time interval more accurate than that of the millisecond.
			Sleep(time / 1000000);
#elif PLATFORM_LINUX
			timespec t1;
			if (time >= 1000000000)
			{
				t1.tv_sec = time / 1000000000;
				time -= t1.tv_sec * 1000000000;
			}
			else
				t1.tv_sec = 0;
			t1.tv_nsec = time;
			timespec t2;
			//std::cout << "\nDelaying " << t1.tv_sec << " seconds " << t1.tv_nsec << " nanoseconds";
			nanosleep(&t1, &t2);

#else
#error delay(time) not implemented!
#endif
		}
	}
}
