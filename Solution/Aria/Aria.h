#pragma once
#include <functional>

namespace aria
{
	struct AriaInitializationParameters
	{
		bool launchGUI = true;
	};

	/** Returns when the aria system stops. */
	void run(const AriaInitializationParameters& parameters);
}