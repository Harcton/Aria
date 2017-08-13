#include "Time.h"
#include "Log.h"
#ifdef SHELL_CODEX
//SHELL inlcudes
#include <time.h>
#endif

#ifdef GHOST_CODEX
//GHOST includes

#endif

namespace codex
{
	namespace time
	{
		void delay(TimeType time)
		{
#ifdef PLATFORM_ARM
			timespec t1;
			t1.tv_sec = 0;
			t1.tv_nsec = (long int)time;
			timespec t2;
			nanosleep(&t1, &t2);
			return;
#endif
#ifdef PLATFORM_PC
			log::error("PC delay delay not yet implemented!");
			return;
#endif
			log::error("codex::time::delay error: unknown platform!");
			return;
		}
	}
}
