#include "Codex.h"
#include "Log.h"
#include <thread>
#ifdef SHELL_CODEX
#include <bcm2835.h>
#endif

namespace codex
{
#ifdef GHOST_CODEX
	const CodexType codexType = CodexType::ghost;
#elif SHELL_CODEX
	const CodexType codexType = CodexType::shell;
#else
#error Invalid codex implementation type
	const CodexType localCodexUser = CodexType::invalid;
#endif
	std::string workingDirectory;

	int initialize(const int argc, const char** argv)
	{
#ifdef SHELL_CODEX
		log::info("Initializing Shell Codex...");

		if (!bcm2835_init())
		{
			log::error("Failed to initialize the bcm 2835 library!");
			return 1;
		}
#endif
#ifdef GHOST_CODEX
		log::info("Initializing Ghost Codex...");
		if (argc > 0)
			codex::workingDirectory = argv[0];
#endif
		
		//Check available integer widths
		if (sizeof(int8_t) != 1 || sizeof(uint8_t) != 1)
			log::warning("8 bit integer width not available!");
		if (sizeof(int16_t) != 2 || sizeof(uint16_t) != 2)
			log::warning("16 bit integer width not available!");
		if (sizeof(int32_t) != 4 || sizeof(uint32_t) != 4)
			log::warning("32 bit integer width not available!");
		if (sizeof(int64_t) != 8 || sizeof(uint64_t) != 8)
			log::warning("64 bit integer width not available!");

		//Check hardware threads
		log::info(std::to_string(std::thread::hardware_concurrency()) + " hardware threads detected.");

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