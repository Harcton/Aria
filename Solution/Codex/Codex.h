#include <string>

namespace codex
{
	int initialize(const int argc, const char** argv);
	void uninitialize();

	extern std::string workingDirectory;
}