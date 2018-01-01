#include "Codex.h"
#include "CodexTime.h"
#include "Protocol.h"
#include "RTTI.h"
#include "Log.h"
#include "SpehsEngine/Core/Core.h"
#include "SpehsEngine/Core/SpehsAssert.h"
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
			codex::log::info("Codex is already initialized!");
			return 1;
		}


		log::info("Initializing Codex...");

#ifdef CODEX_GPIO
		if (bcm2835_init())
		{
			log::info("bcm2835 library successfully initialized.");
		}
		else
		{
			log::error("Failed to initialize the bcm 2835 library!");
			return 1;
		}
#endif
		
		//Print unavailable integer widths
		if (sizeof(int8_t) != 1 || sizeof(uint8_t) != 1)
			log::warning("8 bit integer width not available!");
		if (sizeof(int16_t) != 2 || sizeof(uint16_t) != 2)
			log::warning("16 bit integer width not available!");
		if (sizeof(int32_t) != 4 || sizeof(uint32_t) != 4)
			log::warning("32 bit integer width not available!");
		if (sizeof(int64_t) != 8 || sizeof(uint64_t) != 8)
			log::warning("64 bit integer width not available!");

		//Print hardware thread count
		log::info(std::to_string(std::thread::hardware_concurrency()) + " hardware threads detected.");

		//Print byte order
		log::info("Host system is '" + protocol::getEndiannessAsString(protocol::hostByteOrder) + "' endian.");
		log::info("Network traffic is set to '" + protocol::getEndiannessAsString(protocol::networkByteOrder) + "' endian.");

		//Print clock accuracy
		log::info("Codex time accuracy is " + std::to_string(codex::time::conversionRate::second) + " ticks per second.");

		log::info("Codex initialized.");
		initialized = true;
		return 0;
	}
	void uninitialize()
	{
		if (!initialized)
		{
			codex::log::info("Codex was not initialized!");
			return;
		}

		log::info("Uninitializing Codex...");

#ifdef CODEX_GPIO
		bcm2835_close();
#endif

		log::info("Codex uninitialized.");
	}
}