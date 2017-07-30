#include <iostream>


int main(int argc, char** argv)
{
	std::cout << "\nGhost0 initializing...";

	for (int i = 0; i < argc; i++)
		std::cout << "\n" << argv[i];	

	std::getchar();
}