#include <Codex/Codex.h>
#include <SpehsEngine/Audio/Audio.h>
#include <SpehsEngine/Core/Core.h>
#include <SpehsEngine/GUI/GUI.h>
#include <SpehsEngine/GUI/GUIRectangle.h>
#include <SpehsEngine/Input/Input.h>
#include <SpehsEngine/Rendering/Rendering.h>
#include <SpehsEngine/Core/ApplicationData.h>
#include <SpehsEngine/Input/InputManager.h>
#include <SpehsEngine/Audio/AudioEngine.h>
#include <SpehsEngine/Rendering/TextureManager.h>
#include <SpehsEngine/Rendering/ShaderManager.h>
#include <SpehsEngine/Rendering/BatchManager.h>
#include <SpehsEngine/Rendering/LineDiagram.h>
#include <SpehsEngine/Rendering/Camera2D.h>
#include <SpehsEngine/Rendering/Console.h>
#include <SpehsEngine/Rendering/Window.h>
#include <SpehsEngine/Rendering/GLContext.h>
#include <SpehsEngine/Core/Time.h>
#include <SpehsEngine/Core/RNG.h>

int main(const int argc, const char** argv)
{
	spehs::CoreLib core;
	spehs::AudioLib audio(core);
	spehs::RenderingLib rendering(core);
	spehs::InputLib input(rendering);
	spehs::GUILib gui(input, audio);
	spehs::Window window(500, 500);
	spehs::InputManager inputManager(window);
	spehs::GLContext glContext(window);
	spehs::Camera2D camera(window);
	spehs::TextureManager textureManager(glContext);
	spehs::ShaderManager shaderManager;
	spehs::BatchManager batchManager(window, camera, textureManager, shaderManager, "arm0");
	spehs::time::DeltaTimeSystem deltaTimeSystem;
	spehs::GUIContext guiContext(batchManager, inputManager, deltaTimeSystem);
	spehs::Console console(batchManager, inputManager);
	codex::initialize(argc, argv);

	//Testting initialization here please...
	spehs::LineDiagram diagram(batchManager);
	diagram.setSize(200.0f, 100.0f);
	diagram.setPosition(window.getWidth() / 2 - 0.5f * diagram.getWidth(), window.getHeight() / 2 - 0.5f * diagram.getHeight());

	//Update & render loop
	bool run = true;
	while (run)
	{
		//Spehs update
		deltaTimeSystem.deltaTimeSystemUpdate();
		inputManager.update();
		spehs::audio::AudioEngine::update();
		console.update(deltaTimeSystem.deltaTime);
		if (inputManager.isQuitRequested() || inputManager.isKeyPressed(KEYBOARD_ESCAPE))
			run = false;

		//Test update
		if (inputManager.isKeyDown(KEYBOARD_UP))
			diagram.setCapacity(diagram.getCapacity() + 1);
		if (inputManager.isKeyDown(KEYBOARD_DOWN))
			diagram.setCapacity(std::max(0, (int)diagram.getCapacity() - 1));
		if (inputManager.isKeyDown(KEYBOARD_LEFT))
		{
			diagram.setSize(std::max(0, (int)diagram.getWidth() - 1), diagram.getHeight());
			diagram.setPosition(window.getWidth() / 2 - 0.5f * diagram.getWidth(), window.getHeight() / 2 - 0.5f * diagram.getHeight());
		}
		if (inputManager.isKeyDown(KEYBOARD_RIGHT))
		{
			diagram.setSize(diagram.getWidth() + 1, diagram.getHeight());
			diagram.setPosition(window.getWidth() / 2 - 0.5f * diagram.getWidth(), window.getHeight() / 2 - 0.5f * diagram.getHeight());
		}
		if (inputManager.isCtrlDown())
			diagram.pushBack(sin(spehs::time::getRunTime().asSeconds() * 0.5f) * 100.0f + spehs::rng::random(-50.0f, 50.0f));

		//Render
		window.renderBegin();
		batchManager.render();
		console.render();
		window.renderEnd();
	}

	codex::uninitialize();
	return 0;
}