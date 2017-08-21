#include <assert.h>
#include "CodexTime.h"
#include "Log.h"
#ifdef PLATFORM_LINUX
//Linux includes
#include <time.h>
#endif
#ifdef PLATFORM_WINDOWS
//Windows includes
#include <windows.h>
#endif

namespace codex
{
	namespace time
	{
		void delay(TimeType time)
		{
#ifdef PLATFORM_LINUX

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

#elif PLATFORM_WINDOWS
			assert(time >= 1000000);
			//On windows we don't have - and most likely don't need - a time interval more accurate than that of the millisecond.
			Sleep(time / 1000000);
#else

			static_assert(false, "Platform needs CodexTime implementation!");

#endif
		}
	}
}
