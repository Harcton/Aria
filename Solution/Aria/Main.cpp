//#include <SpehsEngine/Input/InputManager.h>
//#include <SpehsEngine/Rendering/BatchManager.h>
//#include <SpehsEngine/Rendering/TextureManager.h>
//#include <SpehsEngine/Rendering/ShaderManager.h>
//#include <SpehsEngine/Audio/AudioEngine.h>
//#include <SpehsEngine/Rendering/Camera2D.h>
//#include <SpehsEngine/Rendering/Console.h>
//#include <SpehsEngine/Rendering/Window.h>
//#include <SpehsEngine/Rendering/GLContext.h>
//#include <SpehsEngine/Core/Time.h>
//#include <SpehsEngine/Core/Core.h>
//#include <SpehsEngine/Audio/Audio.h>
//#include <SpehsEngine/Rendering/Rendering.h>
//#include <SpehsEngine/Input/Input.h>
//#include <SpehsEngine/GUI/GUI.h>
//#include <SpehsEngine/GUI/GUIRectangle.h>
#include "SpehsEngine/Core/Core.h"
#include "SpehsEngine/Net/Net.h"
#include "SpehsEngine/Net/SocketTCP.h"
#include "SpehsEngine/Net/IOService.h"
#include "SpehsEngine/Net/Aria.h"
#include <atomic>
#include <functional>


int main(const int argc, const char** argv)
{
	//spehs::CoreLib core;
	//spehs::AudioLib audio(core);
	//spehs::RenderingLib rendering(core);
	//spehs::InputLib input(rendering);
	//spehs::GUILib gui(input, audio);
	//spehs::Window window(500, 500);
	//spehs::InputManager inputManager(window);
	//spehs::GLContext glContext(window);
	//spehs::Camera2D camera(window);
	//spehs::TextureManager textureManager(glContext);
	//spehs::ShaderManager shaderManager;
	//spehs::BatchManager batchManager(window, camera, textureManager, shaderManager, "arm0");
	//spehs::time::DeltaTimeSystem deltaTimeSystem;
	//spehs::GUIContext guiContext(batchManager, inputManager, deltaTimeSystem);
	//spehs::Console console(batchManager, inputManager);
	spehs::CoreLib coreLib;
	spehs::NetLib netLib(coreLib);

	spehs::net::PortType port = spehs::net::defaultAriaPort;
	if (argc > 1)
		port = std::atoi(argv[1]);

	spehs::aria::Server aria;
	aria.start(port);
	while (aria.isRunning())
	{
	}

	return 0;
}