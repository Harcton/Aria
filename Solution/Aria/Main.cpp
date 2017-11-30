#include <SpehsEngine/SpehsEngine.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/BatchManager.h>
#include <SpehsEngine/AudioEngine.h>
#include <SpehsEngine/Camera2D.h>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/Time.h>
#include <Codex/Codex.h>
#include <Codex/CodexTime.h>
#include <Codex/SocketTCP.h>
#include <Codex/IOService.h>
#include <Codex/Aria.h>
#include <atomic>
#include <functional>


int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);

	codex::protocol::PortType port = codex::protocol::defaultAriaPort;
	if (argc > 1)
		port = std::atoi(argv[1]);

	codex::aria::Server aria;
	aria.start(port);
	while (aria.isRunning())
	{
	}

	codex::uninitialize();
	return 0;
}

//spehs::initialize("Aria");
//
////Spehs engine
//spehs::Camera2D camera;
//spehs::BatchManager batchManager(&camera, "ghostbox");
//spehs::time::DeltaTimeSystem deltaTimeSystem;
//
//while (aria.isRunning())
//{
//	//Spehs update
//	deltaTimeSystem.deltaTimeSystemUpdate();
//	inputManager->update();
//	spehs::audio::AudioEngine::update();
//	spehs::console::update(deltaTimeSystem.deltaTime);
//	if (inputManager->isQuitRequested() || inputManager->isKeyPressed(KEYBOARD_ESCAPE))
//		aria.stop();
//
//	//Render
//	spehs::getMainWindow()->renderBegin();
//	batchManager.render();
//	spehs::console::render();
//	spehs::getMainWindow()->renderEnd();
//}
//
//spehs::uninitialize();