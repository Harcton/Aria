#include <SpehsEngine/Core/ApplicationData.h>
#include <SpehsEngine/Input/InputManager.h>
#include <SpehsEngine/Audio/AudioEngine.h>
#include <SpehsEngine/Audio/Audio.h>
#include <SpehsEngine/Core/Core.h>
#include <SpehsEngine/Input/Input.h>
#include <SpehsEngine/Rendering/Rendering.h>
#include <SpehsEngine/GUI/GUI.h>
#include <SpehsEngine/Rendering/TextureManager.h>
#include <SpehsEngine/Rendering/ShaderManager.h>
#include <SpehsEngine/Rendering/BatchManager.h>
#include <SpehsEngine/Rendering/Camera2D.h>
#include <SpehsEngine/Rendering/Polygon.h>
#include <SpehsEngine/Rendering/Console.h>
#include <SpehsEngine/Rendering/Window.h>
#include <SpehsEngine/Rendering/GLContext.h>
#include <SpehsEngine/Core/Time.h>
#include <SpehsEngine/Core/RNG.h>
#include <SpehsEngine/GUI/GUIRectangle.h>
#include <Codex/Device/Servo.h>
#include <Codex/Sync/SyncManager.h>
#include <Codex/Protocol.h>
#include <Codex/Codex.h>
#include <Codex/Aria.h>

#include "SpehsEngine/Core/ApplicationData.h"


int main(const int argc, const char** argv)
{
	//Appvars
	spehs::Appvars appvars("Input");
	spehs::Appvar<int> windowWidth(appvars, "Input", "Window width", 900);
	spehs::Appvar<int> windowHeight(appvars, "Input", "Window height", 600);

	//Initialize libraries
	spehs::CoreLib core;
	spehs::RenderingLib rendering(core);
	spehs::InputLib input(rendering);
	spehs::AudioLib audio(core);
	spehs::GUILib gui(rendering, audio);
	codex::initialize(argc, argv);

	//Required class instances for spehs engine basic stuff
	spehs::Window window(windowWidth, windowHeight);
	spehs::GLContext glContext(window);
	spehs::Camera2D camera(window);
	spehs::TextureManager textureManger(glContext);
	spehs::ShaderManager shaderManager;
	spehs::BatchManager batchManager(window, camera, textureManger, shaderManager, "ghostbox");
	spehs::InputManager inputManager(window);
	spehs::Console console(batchManager, inputManager);
	
	spehs::time::DeltaTimeSystem deltaTimeSystem;
	spehs::GUIContext guiContext(batchManager, inputManager, deltaTimeSystem);
	spehs::GUIRectangle rect(guiContext);
	rect.setString("Det ar gui rect");
	rect.setStringColor(spehs::Color(255, 128, 128));
	rect.setColor(spehs::Color(128, 155, 0));
	rect.setPositionGlobal(window.getWidth() / 2 - rect.getWidth() / 2, window.getHeight() / 2 - rect.getHeight() / 2);

	//Testing initialization here please...
	std::vector<spehs::Polygon*> polygons;

	//Update & render loop
	bool run = true;
	spehs::time::Time deltaTime = 0;
	while (run)
	{
		const spehs::time::Time beginTime = spehs::time::now();

		//Spehs update
		deltaTimeSystem.deltaTimeSystemUpdate();
		inputManager.update();
		spehs::audio::AudioEngine::update();
		console.update(deltaTimeSystem.deltaTime);
		if (inputManager.isQuitRequested() || inputManager.isKeyPressed(KEYBOARD_ESCAPE))
			run = false;
		
		rect.inputUpdate();
		rect.visualUpdate();

		//Test update...
		if (inputManager.isKeyDown(KEYBOARD_SPACE))
		{
			for (size_t i = 0; i < 10; i++)
			{
				polygons.push_back(batchManager.createPolygon(3, 0, 1.0f, 1.0f));
				polygons.back()->setCameraMatrixState(false);
				polygons.back()->setPosition(spehs::rng::random<float>(0.0f, (float)window.getWidth()), spehs::rng::random<float>(0.0f, (float)window.getHeight()));
			}
		}
		
		//Render
		window.renderBegin();
		batchManager.render();
		console.render("FPS: " + std::to_string((int)(1.0f / deltaTime.asSeconds())));
		window.renderEnd();

		deltaTime = spehs::time::now() - beginTime;
	}

	codex::uninitialize();
	return 0;
}