#include <iostream>
#include "Aria.h"

int main(int argc, char** argv)
{
	aria::AriaInitializationParameters initializationParameters;
	initializationParameters.launchGUI = true;

	std::cout << "\nLaunching Aria...";
	aria::run(initializationParameters);
	std::cout << "\nAria has stopped.";

	std::getchar();
}