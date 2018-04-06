#include <SpehsEngine/Audio/AudioEngine.h>
#include <SpehsEngine/Audio/Audio.h>
#include <SpehsEngine/Core/Core.h>
#include <SpehsEngine/Core/Inifile.h>
#include <SpehsEngine/Core/Time.h>
#include <SpehsEngine/Core/RNG.h>
#include <SpehsEngine/Input/Input.h>
#include <SpehsEngine/Input/InputManager.h>
#include <SpehsEngine/Rendering/Rendering.h>
#include <SpehsEngine/Rendering/Line.h>
#include <SpehsEngine/GUI/GUI.h>
#include <SpehsEngine/Rendering/TextureManager.h>
#include <SpehsEngine/Rendering/ShaderManager.h>
#include <SpehsEngine/Rendering/BatchManager.h>
#include <SpehsEngine/Rendering/Camera2D.h>
#include <SpehsEngine/Rendering/Polygon.h>
#include <SpehsEngine/Rendering/Console.h>
#include <SpehsEngine/Rendering/Window.h>
#include <SpehsEngine/Rendering/GLContext.h>
#include <SpehsEngine/GUI/GUIRectangle.h>
#include "SpehsEngine/Net/Net.h"
#include "SpehsEngine/Sync/Sync.h"
#include "SpehsEngine/GPIO/GPIO.h"
#include "SpehsEngine/GPIO/Device/Servo.h"
#include "SpehsEngine/Sync/SyncManager.h"
#include "SpehsEngine/GPIO/Manipulator.h"
#include "SpehsEngine/Net/Protocol.h"
#include "SpehsEngine/Net/Aria.h"
#include <thread>

//void runWindow(std::string windowName, spehs::Appvars& appvars)
void runWindow(std::string windowName, spehs::Inifile* ptr)
{
	spehs::Inifile& inifile = *ptr;
	spehs::Inivar<unsigned>& windowWidth = inifile.get(windowName, "width", 900u);
	spehs::Inivar<unsigned>& windowHeight = inifile.get(windowName, "height", 600u);

	spehs::Window window(windowWidth, windowHeight);
	window.setClearColor(spehs::Color(64, 0, 0));
	window.setTitle(windowName.c_str());
	spehs::Camera2D camera(window);
	spehs::ShaderManager shaderManager;
	spehs::BatchManager batchManager(window, shaderManager, camera, "ghostbox");
	spehs::InputManager inputManager(window);
	spehs::Console console;
	spehs::ConsoleVisualizer consoleVisualizer(console, inputManager, batchManager);
	spehs::time::DeltaTimeSystem deltaTimeSystem;
	spehs::GUIContext guiContext(batchManager, inputManager, deltaTimeSystem);

	//Testing initialization here please...
	std::vector<spehs::Polygon*> polygons;
	for (size_t i = 0; i < 24; i++)
	{
		console.log("log " + std::to_string(i));
	}
	
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
		consoleVisualizer.update(deltaTimeSystem.deltaTime);
		if (inputManager.isQuitRequested() || inputManager.isKeyPressed(KEYBOARD_ESCAPE))
			run = false;
		camera.update();
		
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
		consoleVisualizer.render("FPS: " + std::to_string((int)(1.0f / deltaTime.asSeconds())));
		window.renderEnd();
		
		deltaTime = spehs::time::now() - beginTime;
	}
}

int main(const int argc, const char** argv)
{
	//Appvars
	spehs::Inifile inifile("ghostbox");
	inifile.read();

	//Initialize libraries
	spehs::CoreLib coreLib;
	spehs::RenderingLib renderingLib(coreLib);
	spehs::InputLib inputLib(renderingLib);
	spehs::AudioLib audioLib(coreLib);
	spehs::GUILib guiLib(renderingLib, audioLib);
	spehs::NetLib netLib(coreLib);
	spehs::SyncLib syncLib(netLib);
	spehs::GPIOLib gpioLib(syncLib);

	const size_t threadCount = 1;
	std::thread* threads[threadCount];
	for (size_t i = 0; i < threadCount; i++)
	{
		const std::string str("Window" + std::to_string(i + 1));
		threads[i] = new std::thread(runWindow, str, &inifile);
	}

	spehs::time::delay(spehs::time::fromSeconds(1.0f));
	inifile.update();

	for (size_t i = 0; i < threadCount; i++)
	{
		threads[i]->join();
		delete threads[i];
	}

	return 0;
}
