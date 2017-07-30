#include <Codex/Log.h>
#include <boost/asio.hpp>
#include <iostream>


int main()
{
	boost::asio::io_service ioService;
	boost::asio::ip::tcp::socket socketTCP(ioService);
	std::cout << "\nShell0 initializing2...";

	//codex::log::info("Codex test");
	
	std::getchar();
	return 0;
}
//int main(int argc, char** argv)
//{
//	std::cout << "\nShell0 initializing...";
//
//	//codex::log::info("Codex test");
//
//	for (int i = 0; i < argc; i++)
//		std::cout << "\n" << argv[i];
//
//	std::getchar();
//	return 0;
//}