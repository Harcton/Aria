#include <iostream>
#include <Codex/Log.h>
#include <string>

int main(int argc, char** argv)
{
	std::cout << "\nGhost0 initializing...";

	codex::log::info((const unsigned char*)std::to_string(argc).c_str());
	for (int i = 0; i < argc; i++)
		std::cout << "\n" << argv[i];
	std::getchar();
}