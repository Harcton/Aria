#include <string>

namespace codex
{
	int initialize(const int argc, const char** argv);

	bool isInitialized();

	void uninitialize();

	extern std::string workingDirectory;
}