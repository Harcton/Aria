#include <Codex/Codex.h>
#include "Ghost0.h"


int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);
	
	Ghost0 ghost0;
	ghost0.start(argc, argv);

	codex::uninitialize();
}