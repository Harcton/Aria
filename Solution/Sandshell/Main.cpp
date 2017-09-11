#include <Codex/Codex.h>
#include <Codex/Log.h>
#include <iostream>

int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);

	codex::log::info("Sandshell");

	codex::uninitialize();
}