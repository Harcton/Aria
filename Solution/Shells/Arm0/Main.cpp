#include <Codex/Codex.h>
#include <boost/system/error_code.hpp>//->Works
#include <Codex/Device/Servo.h>
#include <Codex/Sync/SyncManager.h>
#include <Codex/Aria.h>
#include <SpehsEngine/Core/Core.h>

int main(const int argc, const char** argv)
{
	spehs::CoreLib core;
	codex::initialize(argc, argv);

	bool keepRunning = true;
	while (keepRunning)
	{
		codex::IOService ioService;
		codex::SocketTCP socket(ioService);
		codex::aria::Connector connector(socket, "arm0Shell", "arm0Ghost", 41623);
		if (connector.enter(codex::protocol::Endpoint("192.168.10.51", codex::protocol::defaultAriaPort)))
			spehs::log::info("yay!");
		else
			spehs::log::info("nay!");

		codex::sync::Manager syncManager(socket);
		syncManager.registerType<codex::device::ServoGhost, codex::device::ServoShell>();
		if (syncManager.initialize())
		{
			spehs::time::Time deltaTime = 0;
			while (socket.isConnected())
			{
				const spehs::time::Time beginTime = spehs::time::now();
				socket.update();
				syncManager.update(deltaTime);
				deltaTime = spehs::time::now() - beginTime;
			}
		}
	}

	codex::uninitialize();
	std::getchar();
	return 0;
}
//#include <Codex/Log.h>
//#include <Codex/Codex.h>
//#include "Shell0.h"
//
//
///*
//	Arguments:
//	0: path
//	1: connect address
//	2. connect port
//
//	or
//
//	0: path
//	1. receive port
//*/
//int main(const int argc, const char** argv)
//{
//	codex::initialize(argc, argv);
//
//	while (1)
//	{
//		Shell0 shell0;
//		shell0.start(argc, argv);
//	}
//
//	codex::uninitialize();
//	return 0;
//}
//
//
///*
////TEST BASE
//
//std::atomic<int> strength(0);
//std::atomic<bool> keepRunningInput(false);
//void runInput()
//{
//keepRunningInput = true;
//while (keepRunningInput)
//{
//spehs::log::info("Input strength:\n>");
//int val;
//std::cin >> val;
//strength = val;
//}
//}
//
////Main
//std::thread inputThread(runInput);
//codex::gpio::Pin pin = codex::gpio::pin_11;
//codex::gpio::setPinAsOutput(pin);
//while (strength >= 0)
//{
//spehs::time::Time interval = spehs::time::fromMilliseconds(1);
//spehs::time::delay(interval);
//}
//inputThread.join();
//
//*/