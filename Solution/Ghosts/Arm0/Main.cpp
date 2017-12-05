//SPEHS ENGINE
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
//CODEX
#include <Codex/Aria.h>
#include <Codex/Codex.h>
#include <Codex/IOService.h>
#include <Codex/SocketTCP.h>
#include <Codex/Device/Servo.h>
#include <Codex/Sync/SyncManager.h>
#include <GhostCodex/ServoCreator.h>
//ARM0



int main(const int argc, const char** argv)
{
	//Codex init
	codex::initialize(argc, argv);
	
	//Spehs engine init
	spehs::initialize("Ghostbox");
	spehs::Camera2D camera;
	spehs::BatchManager batchManager(&camera, "ghostbox");
	spehs::time::DeltaTimeSystem deltaTimeSystem;
	
	//Arm0 init
	codex::time::delay(codex::time::seconds(1.0f));
	codex::IOService ioService;
	codex::SocketTCP socket(ioService);
	codex::aria::Connector connector(socket, "ghostbox1", "ghostbox2", 41623);
	if (connector.enter(codex::protocol::Endpoint("192.168.10.51", codex::protocol::defaultAriaPort)))
		codex::log::info("yay!");
	else
	{
		codex::log::info("nay!");
		std::getchar();
		return 1;
	}

	codex::sync::Manager syncManager(socket);
	syncManager.registerType<codex::device::ServoGhost, codex::device::ServoShell>();
	if (!syncManager.initialize())
	{
		std::getchar();
		return 1;
	}
	codex::ServoCreator servoCreator(batchManager, syncManager);
	
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
		servoCreator.setPositionGlobal(spehs::ApplicationData::getWindowWidthHalf() - servoCreator.getWidth() / 2, spehs::ApplicationData::getWindowHeightHalf() - servoCreator.getHeight() / 2);
		spehs::GUIRectangle::InputUpdateData guiInputUpdateData(inputManager->getMouseCoords(), deltaTimeSystem.deltaTime);
		servoCreator.inputUpdate(guiInputUpdateData);
		servoCreator.visualUpdate();

		//Render
		spehs::getMainWindow()->renderBegin();
		batchManager.render();
		spehs::console::render();
		spehs::getMainWindow()->renderEnd();
	}

	spehs::uninitialize();		
	codex::uninitialize();
}