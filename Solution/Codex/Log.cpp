#include <iostream>
#include "Log.h"

namespace codex
{
	namespace log
	{
		void info(const std::string& message)
		{
			info(message.c_str());
		}
		void warning(const std::string& message)
		{
			warning(message.c_str());
		}
		void error(const std::string& message)
		{
			error(message.c_str());
		}
		void info(const char* message)
		{
			std::cout << "\n" << message;
		}
		void warning(const char* message)
		{
			std::cout << "\nWarning: " << message;
		}
		void error(const char* message)
		{
			std::cout << "\nError: " << message << "\nPress enter to continue...\n";
			std::getchar();
		}
	}
}