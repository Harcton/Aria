#include <boost/asio.hpp>
#include <SpehsEngine/SpehsEngine.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/BatchManager.h>
#include <SpehsEngine/AudioEngine.h>
#include <SpehsEngine/StringOperations.h>
#include <SpehsEngine/Camera2D.h>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/Time.h>
#include <Codex/Codex.h>
#include <Codex/CodexTime.h>
#include <Codex/SocketTCP.h>
#include <Codex/IOService.h>
#include <atomic>
#include <functional>



//Server
void onAccept(codex::SocketTCP& socket)
{
	//codex::log::info("Acceptor successfully accepted an incoming connection from " + socket.getRemoteAddress());

	
	//socket.startReceiving(std::bind(&codex::AriaSocketTCP::ghostRequestHandler, &dynamic_cast<codex::AriaSocketTCP&>(socket), std::placeholders::_1));
}
int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);
	//codex::log::info("Launching Aria.");

	////Aria socket
	//codex::IOService ioService;
	//codex::AriaSocketTCP socket(ioService);
	//socket.resizeReceiveBuffer(64000);
	//std::vector<codex::SocketTCP*> sockets;

	////Aria main loop
	//bool running = true;
	//while (running)
	//{
	//	if (!socket.isConnected() && !socket.isAccepting())
	//		socket.startAccepting(codex::protocol::defaultAriaPort, std::bind(&onAccept, std::placeholders::_1));
	//}

	//codex::log::info("Aria has stopped.");
	codex::uninitialize();
	return 0;
}