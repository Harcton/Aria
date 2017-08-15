#include "Time.h"
#include "Log.h"
#ifdef PLATFORM_LINUX
//Linux includes
#include <time.h>
#endif
#ifdef PLATFORM_WINDOWS
//Windows includes

#endif

namespace codex
{
	namespace time
	{
		void delay(TimeType time)
		{
#ifdef PLATFORM_LINUX
			timespec t1;
			t1.tv_sec = 0;
			t1.tv_nsec = (long int)time;
			timespec t2;
			nanosleep(&t1, &t2);
			return;
#endif
#ifdef PLATFORM_WINDOWS
			log::error("Windows delay not yet implemented!");
			return;
#endif
			log::error("codex::time::delay error: unknown platform!");
			return;
		}
	}
}
