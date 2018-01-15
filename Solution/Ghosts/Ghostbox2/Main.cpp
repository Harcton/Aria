#include <Codex/Codex.h>
#include <boost/system/error_code.hpp>//->Works
#include <Codex/Device/Servo.h>
#include <Codex/Sync/SyncManager.h>
#include <Codex/Aria.h>
#include <SpehsEngine/Audio/Audio.h>
#include <SpehsEngine/Core/Core.h>
#include <SpehsEngine/Input/Input.h>
#include <SpehsEngine/Input/InputManager.h>
#include <SpehsEngine/Rendering/Rendering.h>
#include <SpehsEngine/Rendering/Window.h>
#include <SpehsEngine/Rendering/GLContext.h>
#include <SpehsEngine/Rendering/Camera2D.h>
#include <SpehsEngine/Rendering/TextureManager.h>
#include <SpehsEngine/Rendering/ShaderManager.h>
#include <SpehsEngine/Rendering/BatchManager.h>
#include <SpehsEngine/GUI/GUI.h>
#include <SpehsEngine/GUI/GUIRectangle.h>


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

	bool keepRunning = true;
	while (keepRunning)
	{
		codex::IOService ioService;
		codex::SocketTCP socket(ioService);
		codex::aria::Connector connector(socket, "arm0Shell", "arm0Ghost", 41623);
		if (connector.enter(codex::protocol::Endpoint("192.168.10.51", codex::protocol::defaultAriaPort)))
			spehs::log::info("yay!");
		else
			spehs::log::info("nay!");

		codex::sync::Manager syncManager(socket);
		syncManager.registerType<codex::device::ServoGhost, codex::device::ServoShell>();
		if (syncManager.initialize())
		{
			spehs::time::Time deltaTime = 0;
			while (socket.isConnected())
			{
				const spehs::time::Time beginTime = spehs::time::now();
				socket.update();
				syncManager.update(deltaTime);
				deltaTime = spehs::time::now() - beginTime;
			}
		}
	}

	codex::uninitialize();
	return 0;
}