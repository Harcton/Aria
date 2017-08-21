#include <iostream>
#include <Codex/Log.h>
#include "Aria.h"

int main(int argc, char** argv)
{
	codex::log::info("Launching Aria...");
	aria::run();
	codex::log::info("Aria has stopped.");

	std::getchar();
}