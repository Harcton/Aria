//SPEHS ENGINE
#include <SpehsEngine/Core/ApplicationData.h>
#include <SpehsEngine/Input/InputManager.h>
#include <SpehsEngine/Audio/AudioEngine.h>
#include <SpehsEngine/Audio/Audio.h>
#include <SpehsEngine/Core/Core.h>
#include <SpehsEngine/GUI/GUI.h>
#include <SpehsEngine/Input/Input.h>
#include <SpehsEngine/Rendering/Rendering.h>
#include <SpehsEngine/Rendering/BatchManager.h>
#include <SpehsEngine/Rendering/Camera2D.h>
#include <SpehsEngine/Rendering/Console.h>
#include <SpehsEngine/Input/Window.h>
#include <SpehsEngine/Core/Time.h>
#include <SpehsEngine/Core/RNG.h>
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
	spehs::core::initialize();
	spehs::audio::initialize();
	spehs::input::initialize();
	spehs::rendering::initialize();
	spehs::gui::initialize();
	spehs::Camera2D camera;
	spehs::BatchManager batchManager(&camera, "arm0");
	spehs::time::DeltaTimeSystem deltaTimeSystem;
	
	//Arm0 init
	spehs::time::delay(spehs::time::fromSeconds(1.0f));
	codex::IOService ioService;
	codex::SocketTCP socket(ioService);
	codex::aria::Connector connector(socket, "arm0Ghost", "arm0Shell", 41623);
	if (connector.enter(codex::protocol::Endpoint("192.168.10.51", codex::protocol::defaultAriaPort)))
		spehs::log::info("yay!");
	else
	{
		spehs::log::info("nay!");
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
		syncManager.update((spehs::time::Time)deltaTimeSystem.deltaTime.value);
		servoCreator.setPositionGlobal(spehs::ApplicationData::getWindowWidthHalf() - servoCreator.getWidth() / 2, spehs::ApplicationData::getWindowHeightHalf() - servoCreator.getHeight() / 2);
		spehs::GUIRectangle::InputUpdateData guiInputUpdateData(inputManager->getMouseCoords(), deltaTimeSystem.deltaTime);
		servoCreator.inputUpdate(guiInputUpdateData);
		servoCreator.visualUpdate();

		//Render
		spehs::input::getMainWindow()->renderBegin();
		batchManager.render();
		spehs::console::render();
		spehs::input::getMainWindow()->renderEnd();
	}

	spehs::gui::uninitialize();
	spehs::rendering::uninitialize();
	spehs::input::uninitialize();
	spehs::audio::uninitialize();
	spehs::core::uninitialize();
	codex::uninitialize();
}