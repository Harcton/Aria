#include <atomic>
#include <functional>
#include <boost/asio.hpp>
#include <SpehsEngine/SpehsEngine.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/BatchManager.h>
#include <SpehsEngine/AudioEngine.h>
#include <SpehsEngine/Camera2D.h>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/Time.h>
#include <Codex/SocketTCP.h>
#include <Codex/Acceptor.h>
#include "Aria.h"
#ifdef _WIN32
#include <Windows.h>//NOTE: must be included after boost asio stuff...
#endif


namespace aria
{


	void onAccept(const bool result, codex::SocketTCP& socket)
	{
		if (result)
		{
			codex::log::info("Acceptor successfully accepted an incoming connection: " + socket.getRemoteEndpoint().address().to_string());
		}
	}

	void run(const AriaInitializationParameters& parameters)
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

		codex::SocketTCP socket;
		codex::Acceptor acceptor;
		socket.startAccepting(acceptor, std::bind(&onAccept, std::placeholders::_1, std::placeholders::_2));

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
		spehs::uninitialize();
		return;
	}
}