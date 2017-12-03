#include <Codex/Codex.h>
#include <SpehsEngine/ApplicationData.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/AudioEngine.h>
#include <SpehsEngine/SpehsEngine.h>
#include <SpehsEngine/BatchManager.h>
#include <SpehsEngine/Camera2D.h>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/Time.h>
#include <SpehsEngine/RNG.h>

#include <Codex/Device/Servo.h>
#include <Codex/Sync/SyncManager.h>
#include <Codex/Protocol.h>
#include <Codex/Aria.h>
#include <Codex/RTTI.h>




#include <Codex/GPIO.h>
int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);

	codex::time::delay(codex::time::seconds(1.0f));
	codex::IOService ioService;
	codex::SocketTCP socket(ioService);
	codex::aria::Connector connector(socket, "ghostbox2", "ghostbox1", 41623);
	if (connector.enter(codex::protocol::Endpoint("192.168.10.51", codex::protocol::defaultAriaPort)))
		codex::log::info("yay!");
	else
	{
		codex::log::info("nay!");
		std::getchar();
		return 1;
	}

	//Required class instances for spehs engine basic stuff
	spehs::initialize("Ghostbox");
	spehs::Camera2D camera;
	spehs::BatchManager batchManager(&camera, "ghostbox");
	spehs::time::DeltaTimeSystem deltaTimeSystem;

	//Testing initialization here please...
	codex::sync::Manager syncManager(socket);
	syncManager.registerType<codex::device::ServoGhost, codex::device::ServoShell>();
	if (!syncManager.initialize())
	{
		std::getchar();
		return 1;
	}

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

		//Test update...
		socket.update();
		syncManager.update((codex::time::TimeType)deltaTimeSystem.deltaTime.value);

		//Render
		spehs::getMainWindow()->renderBegin();
		batchManager.render();
		spehs::console::render();
		spehs::getMainWindow()->renderEnd();
	}

	codex::uninitialize();
	return 0;
}