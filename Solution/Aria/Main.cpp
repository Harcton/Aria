#include <iostream>
#include <Codex/Log.h>
#include "Aria.h"

int main(int argc, char** argv)
{
	aria::AriaInitializationParameters initializationParameters;
	initializationParameters.launchGUI = true;

	std::cout << "\nLaunching Aria...";
	codex::log::info((const unsigned char*)"codex test");
	aria::run(initializationParameters);
	std::cout << "\nAria has stopped.";

	std::getchar();
}