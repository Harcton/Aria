#include <Codex/Codex.h>
#include <SpehsEngine/Audio/Audio.h>
#include <SpehsEngine/Core/Core.h>
#include <SpehsEngine/GUI/GUI.h>
#include <SpehsEngine/Input/Input.h>
#include <SpehsEngine/Rendering/Rendering.h>
#include <SpehsEngine/Core/ApplicationData.h>
#include <SpehsEngine/Input/InputManager.h>
#include <SpehsEngine/Audio/AudioEngine.h>
#include <SpehsEngine/Rendering/BatchManager.h>
#include <SpehsEngine/Rendering/LineDiagram.h>
#include <SpehsEngine/Rendering/Camera2D.h>
#include <SpehsEngine/Rendering/Console.h>
#include <SpehsEngine/Input/Window.h>
#include <SpehsEngine/Core/Time.h>
#include <SpehsEngine/Core/RNG.h>

int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);
	spehs::core::initialize();
	spehs::audio::initialize();
	spehs::input::initialize();
	spehs::rendering::initialize();
	spehs::gui::initialize();

	//Required class instances for spehs engine basic stuff
	spehs::Camera2D camera;
	spehs::BatchManager batchManager(&camera, "ghostbox");
	spehs::time::DeltaTimeSystem deltaTimeSystem;

	//Testting initialization here please...
	spehs::LineDiagram diagram(batchManager);
	diagram.setSize(200.0f, 100.0f);
	diagram.setPosition(spehs::ApplicationData::getWindowWidthHalf() - 0.5f * diagram.getWidth(), spehs::ApplicationData::getWindowHeightHalf() - 0.5f * diagram.getHeight());

	//Update & render loop
	bool run = true;
	while (run)
	{
		//Spehs update
		deltaTimeSystem.deltaTimeSystemUpdate();
		inputManager->update();
		spehs::audio::AudioEngine::update();
		spehs::console::update(deltaTimeSystem.deltaTime);
		if (inputManager->isQuitRequested() || inputManager->isKeyPressed(KEYBOARD_ESCAPE))
			run = false;

		//Test update
		if (inputManager->isKeyDown(KEYBOARD_UP))
			diagram.setCapacity(diagram.getCapacity() + 1);
		if (inputManager->isKeyDown(KEYBOARD_DOWN))
			diagram.setCapacity(std::max(0, (int)diagram.getCapacity() - 1));
		if (inputManager->isKeyDown(KEYBOARD_LEFT))
		{
			diagram.setSize(std::max(0, (int)diagram.getWidth() - 1), diagram.getHeight());
			diagram.setPosition(spehs::ApplicationData::getWindowWidthHalf() - 0.5f * diagram.getWidth(), spehs::ApplicationData::getWindowHeightHalf() - 0.5f * diagram.getHeight());
		}
		if (inputManager->isKeyDown(KEYBOARD_RIGHT))
		{
			diagram.setSize(diagram.getWidth() + 1, diagram.getHeight());
			diagram.setPosition(spehs::ApplicationData::getWindowWidthHalf() - 0.5f * diagram.getWidth(), spehs::ApplicationData::getWindowHeightHalf() - 0.5f * diagram.getHeight());
		}
		if (inputManager->isCtrlDown())
			diagram.pushBack(sin(spehs::time::getRunTime().asSeconds() * 0.5f) * 100.0f + spehs::rng::random(-50.0f, 50.0f));

		//Render
		spehs::input::getMainWindow()->renderBegin();
		batchManager.render();
		spehs::console::render();
		spehs::input::getMainWindow()->renderEnd();
	}

	spehs::gui::uninitialize();
	spehs::rendering::uninitialize();
	spehs::input::uninitialize();
	spehs::audio::uninitialize();
	spehs::core::uninitialize();
	codex::uninitialize();
	return 0;
}