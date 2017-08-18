#include <iostream>
#include <Codex/Log.h>
#include <SpehsEngine/SpehsEngine.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/BatchManager.h>
#include <SpehsEngine/AudioEngine.h>
#include <SpehsEngine/Camera2D.h>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/Time.h>
#include <string>

int main(int argc, char** argv)
{
	codex::log::info("Ghost0 initializing...");

	codex::log::info(std::to_string(argc));
	for (int i = 0; i < argc; i++)
		std::cout << "\n" << argv[i];
	
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
	codex::log::info("Ghost0 stopped.");
	std::getchar();
}