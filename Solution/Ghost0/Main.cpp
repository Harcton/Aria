#include <iostream>
#include <Codex/Log.h>


int main(int argc, char** argv)
{
	std::cout << "\nGhost0 initializing...";

	codex::log::info((const unsigned char*)"codex test4");
	for (int i = 0; i < argc; i++)
		std::cout << "\n" << argv[i];	

	std::getchar();
}