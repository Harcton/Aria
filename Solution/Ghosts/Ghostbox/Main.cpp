#include <Codex/Codex.h>
#include <SpehsEngine/Core/ApplicationData.h>
#include <SpehsEngine/Input/InputManager.h>
#include <SpehsEngine/Audio/AudioEngine.h>
#include <SpehsEngine/Audio/Audio.h>
#include <SpehsEngine/Core/Core.h>
#include <SpehsEngine/Input/Input.h>
#include <SpehsEngine/Rendering/Rendering.h>
#include <SpehsEngine/GUI/GUI.h>
#include <SpehsEngine/Rendering/BatchManager.h>
#include <SpehsEngine/Rendering/Camera2D.h>
#include <SpehsEngine/Rendering/Polygon.h>
#include <SpehsEngine/Rendering/Console.h>
#include <SpehsEngine/Input/Window.h>
#include <SpehsEngine/Core/Time.h>
#include <SpehsEngine/Core/RNG.h>

#include <Codex/Device/Servo.h>
#include <Codex/Sync/SyncManager.h>
#include <Codex/Protocol.h>
#include <Codex/Aria.h>
#include <Codex/RTTI.h>





int main(const int argc, const char** argv)
{
	spehs::core::initialize();
	spehs::input::initialize();
	spehs::rendering::initialize();
	spehs::gui::initialize();
	spehs::audio::initialize();
	codex::initialize(argc, argv);
	
	//Required class instances for spehs engine basic stuff
	spehs::Camera2D camera;
	spehs::BatchManager batchManager(&camera, "ghostbox");
	spehs::time::DeltaTimeSystem deltaTimeSystem;
	
	//Testing initialization here please...
	std::vector<spehs::Polygon*> polygons;

	//Update & render loop
	bool run = true;
	codex::time::TimeType deltaTime = 0;
	while (run)
	{
		const codex::time::TimeType beginTime = codex::time::now();

		//Spehs update
		deltaTimeSystem.deltaTimeSystemUpdate();
		inputManager->update();
		spehs::audio::AudioEngine::update();
		spehs::console::update(deltaTimeSystem.deltaTime);
		if (inputManager->isQuitRequested() || inputManager->isKeyPressed(KEYBOARD_ESCAPE))
			run = false;

		//Test update...
		if (inputManager->isKeyDown(KEYBOARD_SPACE))
		{
			for (size_t i = 0; i < 10; i++)
			{
				polygons.push_back(batchManager.createPolygon(3, 0, 1.0f, 1.0f));
				polygons.back()->setCameraMatrixState(false);
				polygons.back()->setPosition(spehs::rng::random<float>(0.0f, (float)spehs::ApplicationData::getWindowWidth()), spehs::rng::random<float>(0.0f, (float)spehs::ApplicationData::getWindowHeight()));
			}
		}
		
		//Render
		spehs::input::getMainWindow()->renderBegin();
		batchManager.render();
		spehs::console::render("FPS: " + std::to_string((int)(1.0f / codex::time::toSeconds(deltaTime))));
		spehs::input::getMainWindow()->renderEnd();

		deltaTime = codex::time::now() - beginTime;
	}

	codex::uninitialize();
	spehs::gui::uninitialize();
	spehs::rendering::uninitialize();
	spehs::input::uninitialize();
	spehs::audio::uninitialize();
	spehs::core::uninitialize();
	return 0;
}