#include <iostream>
#include "Log.h"

namespace codex
{
	namespace log
	{
		void info(const unsigned char* message)
		{
			std::cout << "\n" << message;
		}
		void warning(const unsigned char* message)
		{
			std::cout << "\nWarning: " << message;
		}
		void error(const unsigned char* message)
		{
			std::cout << "\nError: " << message << "\nPress enter to continue...\n";
			std::getchar();
		}
	}
}