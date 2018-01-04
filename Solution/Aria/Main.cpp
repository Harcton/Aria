#include <SpehsEngine/Input/InputManager.h>
#include <SpehsEngine/Rendering/BatchManager.h>
#include <SpehsEngine/Audio/AudioEngine.h>
#include <SpehsEngine/Rendering/Camera2D.h>
#include <SpehsEngine/Rendering/Console.h>
#include <SpehsEngine/Input/Window.h>
#include <SpehsEngine/Core/Time.h>
#include <Codex/Codex.h>
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