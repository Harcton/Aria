#include <Codex/Codex.h>
#include <SpehsEngine/ApplicationData.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/AudioEngine.h>
#include <SpehsEngine/SpehsEngine.h>
#include <SpehsEngine/BatchManager.h>
#include <SpehsEngine/LineDiagram.h>
#include <SpehsEngine/Camera2D.h>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/Time.h>
#include <SpehsEngine/RNG.h>


int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);
	spehs::initialize("Ghostbox");

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
		spehs::getMainWindow()->renderBegin();
		batchManager.render();
		spehs::console::render();
		spehs::getMainWindow()->renderEnd();
	}

	codex::uninitialize();
	return 0;
}