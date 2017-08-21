#include <atomic>
#include <functional>
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
#include <Codex/SocketTCP.h>
#include <Codex/IOService.h>
#include <Codex/CodexTime.h>
#include "Aria.h"
#ifdef _WIN32
#include <Windows.h>//NOTE: must be included after boost asio stuff...
#endif

//Shell
std::atomic<bool> keepRunningShell(true);
bool clientReceiveHandler(codex::protocol::ReadBuffer& buffer)
{
	codex::log::info("Client receive handler received " + std::to_string(buffer.getCapacity()) + " bytes");
	for (size_t i = 0; i < buffer.getWrittenSize(); i++)
	{
		uint8_t byte;
		buffer.read(&byte, sizeof(byte));
		codex::log::info(spehs::toHexString(byte));
	}
	return true;
}
void runShell()
{
	//Buffer
	uint64_t someData = 0x0123456789ABCDEF;
	codex::protocol::WriteBuffer writeBuffer(codex::protocol::Endianness::inverted);
	writeBuffer.write(&someData, sizeof(someData));
	writeBuffer.write(&someData, sizeof(someData));

	//Shell socket
	codex::SocketTCP shellSocket;
	shellSocket.resizeReceiveBuffer(64000);
	if (shellSocket.connect("192.168.10.52", 41623))
		codex::log::info("Successfully connected the tcp socket!");
	if (shellSocket.startReceiving(std::bind(&clientReceiveHandler, std::placeholders::_1)))
		codex::log::info("TCP socket has began successfully receiving data!");
	//codex::time::delay(codex::time::milliseconds(500));
	if (shellSocket.sendPacket(writeBuffer))
		codex::log::info("TCP socket successfully sent a packet!");

	//Loop
	while (keepRunningShell)
	{
		//Blocks
	}
}

namespace aria
{
	//Server
	bool serverReceiveHandler(codex::protocol::ReadBuffer& buffer)
	{
		codex::log::info("Server receive handler received " + std::to_string(buffer.getWrittenSize()) + " bytes");
		for (size_t i = 0; i < buffer.getWrittenSize(); i++)
		{
			uint8_t byte;
			buffer.read(&byte, sizeof(byte));
			codex::log::info(spehs::toHexString(byte));
		}
		return true;
	}
	void onAccept(codex::SocketTCP& socket)
	{
		codex::log::info("Acceptor successfully accepted an incoming connection from " + socket.getRemoteAddress());
		socket.startReceiving(std::bind(&serverReceiveHandler, std::placeholders::_1));
	}
	void run()
	{
		if (false)
		{
#ifdef _WIN32
			// additional information
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			// set the size of the structures
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));
			// start the program up
			CreateProcess("E:/Ohjelmointi/Projects/Aria/Solution/bin/Ghost0.exe",   // the path
				"param0",		// Command line
				NULL,           // Process handle not inheritable
				NULL,           // Thread handle not inheritable
				FALSE,          // Set handle inheritance to FALSE
				CREATE_NEW_CONSOLE,//Creation flags
				NULL,           // Use parent's environment block
				NULL,           // Use parent's starting directory 
				&si,            // Pointer to STARTUPINFO structure
				&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
			);
			// Close process and thread handles.
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
#endif
		}
		
		//IO service
		std::shared_ptr<codex::IOService> ioService(new codex::IOService());

		//Aria socket
		codex::SocketTCP ariaSocket(ioService);
		ariaSocket.resizeReceiveBuffer(64000);
		ariaSocket.startAccepting(41623, std::bind(&onAccept, std::placeholders::_1));
		
		//Shell emulation...
		std::thread shellThread(&runShell);

		//Aria main loop
		spehs::initialize("Aria");
		spehs::Camera2D camera;
		spehs::BatchManager batchManager(&camera, "Aria");
		bool running = true;
		while (running)
		{
			spehs::time::update();
			inputManager->update();
			spehs::audio::AudioEngine::update();
			spehs::console::update();
			if (inputManager->isQuitRequested() || inputManager->isKeyPressed(KEYBOARD_ESCAPE))
				running = false;
			spehs::getMainWindow()->renderBegin();
			batchManager.render();
			spehs::console::render();
			spehs::getMainWindow()->renderEnd();
		}

		//Stop shell
		keepRunningShell = false;
		shellThread.join();

		spehs::uninitialize();
		return;
	}
}