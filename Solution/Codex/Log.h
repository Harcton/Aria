#pragma once
#include <string>

namespace codex
{
	namespace log
	{
		void info(const std::string& message);
		void warning(const std::string& message);
		void error(const std::string& message);
		void info(const char* message);
		void warning(const char* message);
		void error(const char* message);
	}
}