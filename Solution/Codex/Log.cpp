#include <iostream>
#include <mutex>
#include "Log.h"

namespace codex
{
	namespace log
	{
		std::mutex mutex;
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
			std::lock_guard<std::mutex> lock(mutex);
			std::cout << message << std::endl;
		}
		void warning(const char* message)
		{
			std::lock_guard<std::mutex> lock(mutex);
			std::cout << "Warning: " << message << std::endl;
		}
		void error(const char* message)
		{
			std::lock_guard<std::mutex> lock(mutex);
			std::cout << "Error: " << message << "\nPress enter to continue..." << std::endl;
			std::getchar();
		}
	}
}