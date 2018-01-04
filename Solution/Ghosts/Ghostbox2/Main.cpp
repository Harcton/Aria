#include <Codex/Codex.h>
#include <boost/system/error_code.hpp>//->Works
#include <Codex/Device/Servo.h>
#include <Codex/Sync/SyncManager.h>
#include <Codex/Aria.h>
#include <SpehsEngine/Audio/Audio.h>
#include <SpehsEngine/Core/Core.h>
#include <SpehsEngine/Input/Input.h>
#include <SpehsEngine/Rendering/Rendering.h>
#include <SpehsEngine/GUI/GUI.h>


int main(const int argc, const char** argv)
{
	spehs::core::initialize();
	spehs::input::initialize();
	spehs::rendering::initialize();
	spehs::gui::initialize();
	spehs::audio::initialize();
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
	spehs::gui::uninitialize();
	spehs::rendering::uninitialize();
	spehs::input::uninitialize();
	spehs::audio::uninitialize();
	spehs::core::uninitialize();
	std::getchar();
	return 0;
}