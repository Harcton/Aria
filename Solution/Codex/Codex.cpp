#include "Codex.h"
#include "SpehsEngine/Core/Time.h"
#include "Protocol.h"
#include "SpehsEngine/Core/Log.h"
#include "SpehsEngine/Core/Core.h"
#include <thread>
#include <ctime>
#ifdef CODEX_GPIO
#include <bcm2835.h>
#endif


namespace codex
{
	std::string workingDirectory;
	bool initialized = false;

	int initialize(const int argc, const char** argv)
	{
		if (initialized)
		{
			spehs::log::info("Codex is already initialized!");
			return 1;
		}


		spehs::log::info("Initializing Codex...");

#ifdef CODEX_GPIO
		if (bcm2835_init())
		{
			spehs::log::info("bcm2835 library successfully initialized.");
		}
		else
		{
			spehs::log::error("Failed to initialize the bcm 2835 library!");
			return 1;
		}
#endif
		
		//Print unavailable integer widths
		if (sizeof(int8_t) != 1 || sizeof(uint8_t) != 1)
			spehs::log::warning("8 bit integer width not available!");
		if (sizeof(int16_t) != 2 || sizeof(uint16_t) != 2)
			spehs::log::warning("16 bit integer width not available!");
		if (sizeof(int32_t) != 4 || sizeof(uint32_t) != 4)
			spehs::log::warning("32 bit integer width not available!");
		if (sizeof(int64_t) != 8 || sizeof(uint64_t) != 8)
			spehs::log::warning("64 bit integer width not available!");

		//Print hardware thread count
		spehs::log::info(std::to_string(std::thread::hardware_concurrency()) + " hardware threads detected.");

		//Print byte order
		spehs::log::info("Host system is '" + protocol::getEndiannessAsString(protocol::hostByteOrder) + "' endian.");
		spehs::log::info("Network traffic is set to '" + protocol::getEndiannessAsString(protocol::networkByteOrder) + "' endian.");

		//Print clock accuracy
		spehs::log::info("Codex time accuracy is " + std::to_string(spehs::time::conversionRate::second) + " ticks per second.");

		spehs::log::info("Codex initialized.");
		initialized = true;
		return 0;
	}

	bool isInitialized()
	{
		return initialized;
	}

	void uninitialize()
	{
		if (!initialized)
		{
			spehs::log::info("Codex was not initialized!");
			return;
		}

		spehs::log::info("Uninitializing Codex...");

#ifdef CODEX_GPIO
		bcm2835_close();
#endif

		spehs::log::info("Codex uninitialized.");
	}
}