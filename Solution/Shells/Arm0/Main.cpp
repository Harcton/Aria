#include <Codex/Log.h>
#include <Codex/Codex.h>

int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);
	
	codex::uninitialize();
	return 0;
}