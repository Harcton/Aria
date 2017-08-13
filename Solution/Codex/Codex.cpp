#include "Codex.h"
#include "Log.h"
#ifdef SHELL_CODEX
#include <bcm2835.h>
#endif

namespace codex
{
	int initialize()
	{
#ifdef SHELL_CODEX
		log::info("Initializing ShellCodex...");

		if (!bcm2835_init())
		{
			log::error("Failed to initialize the bcm 2835 library!");
			return 1;
		}
#endif
#ifdef GHOST_CODEX
		log::info("Initializing GhostCodex...");
#endif
		return 0;
	}
	void uninitialize()
	{
#ifdef SHELL_CODEX
		log::info("Uninitializing ShellCodex...");
		bcm2835_close();
#endif
#ifdef GHOST_CODEX
		log::info("Uninitializing GhostCodex...");
#endif
	}
}