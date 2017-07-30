#include <Codex/Log.h>
#include <boost/asio.hpp>
#include <iostream>


int main(int argc, char** argv)
{
	boost::asio::io_service ioService;
	boost::asio::ip::tcp::socket socketTCP(ioService);
	std::cout << "\nShell0 initializing...";

	codex::log::info((const unsigned char*)"Codex test");

	std::getchar();
	return 0;
}