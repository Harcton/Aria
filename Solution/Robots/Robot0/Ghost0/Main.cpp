#include <iostream>
#include <Codex/Log.h>
#include <string>

int main(int argc, char** argv)
{
	std::cout << "\nGhost0 initializing...";

	codex::log::info(std::to_string(argc));
	for (int i = 0; i < argc; i++)
		std::cout << "\n" << argv[i];
	std::getchar();
}