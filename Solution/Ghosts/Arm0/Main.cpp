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
#include <SpehsEngine/Rendering/GLContext.h>
#include <SpehsEngine/Rendering/Window.h>
#include <SpehsEngine/Rendering/Console.h>
#include <SpehsEngine/Rendering/ShaderManager.h>
#include <SpehsEngine/Rendering/TextureManager.h>
#include <SpehsEngine/Core/Time.h>
#include <SpehsEngine/Core/RNG.h>
//CODEX
#include <Codex/Aria.h>
#include <Codex/Codex.h>
#include <Codex/IOService.h>
#include <Codex/SocketTCP.h>
#include <Codex/Device/Servo.h>
#include <Codex/Device/PinReader.h>
#include <Codex/Device/PinReaderPWM.h>
#include <Codex/Sync/SyncManager.h>
#include <GhostCodex/ServoCreator.h>
#include <GhostCodex/PinReaderCreator.h>
#include <GhostCodex/PinReaderPWMCreator.h>
//ARM0



int main(const int argc, const char** argv)
{
	//Spehs engine init
	spehs::CoreLib core;
	
	//Codex init
	codex::initialize(argc, argv);
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

	spehs::AudioLib audio(core);
	spehs::RenderingLib rendering(core);
	spehs::InputLib input(rendering);
	spehs::GUILib gui(input, audio);
	spehs::Window window(900, 600); window.setBorderless(true); window.setTitle("Arm0 ghost");
	spehs::InputManager inputManager(window);
	spehs::Camera2D camera(window);
	spehs::ShaderManager shaderManager;
	spehs::BatchManager batchManager(window, shaderManager, camera, "arm0");
	spehs::time::DeltaTimeSystem deltaTimeSystem;
	spehs::GUIContext guiContext(batchManager, inputManager, deltaTimeSystem);
	spehs::Console console(inputManager, &batchManager);

	codex::sync::Manager syncManager(socket);
	syncManager.registerType<codex::device::ServoGhost>(codex::device::ServoShell::getSyncTypeName(), codex::device::ServoShell::getSyncTypeId(), codex::device::ServoShell::getSyncTypeVersion());
	syncManager.registerType<codex::device::PinReaderGhost>(codex::device::PinReaderShell::getSyncTypeName(), codex::device::PinReaderShell::getSyncTypeId(), codex::device::PinReaderShell::getSyncTypeVersion());
	syncManager.registerType<codex::device::PinReaderPWMGhost>(codex::device::PinReaderPWMShell::getSyncTypeName(), codex::device::PinReaderPWMShell::getSyncTypeId(), codex::device::PinReaderPWMShell::getSyncTypeVersion());
	if (!syncManager.initialize())
	{
		std::getchar();
		return 1;
	}
	codex::ServoCreator servoCreator(guiContext, syncManager);
	codex::PinReaderCreator pinReaderCreator(guiContext, syncManager);
	codex::PinReaderPWMCreator pinReaderPWMCreator(guiContext, syncManager);
	
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

		//Test update...
		socket.update();
		syncManager.update((spehs::time::Time)deltaTimeSystem.deltaTime.value);
		const float elementsHeight = servoCreator.getHeight() + pinReaderCreator.getHeight() + pinReaderPWMCreator.getHeight();
		float penPositionY = (0.5f * window.getHeight()) - (0.5f * elementsHeight);
		//Servo creator
		servoCreator.setPositionGlobal(window.getWidth() / 2 - servoCreator.getWidth() / 2, penPositionY);
		penPositionY += servoCreator.getHeight();
		servoCreator.inputUpdate();
		servoCreator.visualUpdate();
		//Pin reader creator
		pinReaderCreator.setPositionGlobal(window.getWidth() / 2 - pinReaderCreator.getWidth() / 2, penPositionY);
		penPositionY += pinReaderCreator.getHeight();
		pinReaderCreator.inputUpdate();
		pinReaderCreator.visualUpdate();
		//Pin reader PWM creator
		pinReaderPWMCreator.setPositionGlobal(window.getWidth() / 2 - pinReaderPWMCreator.getWidth() / 2, penPositionY);
		penPositionY += pinReaderPWMCreator.getHeight();
		pinReaderPWMCreator.inputUpdate();
		pinReaderPWMCreator.visualUpdate();
		
		//Render
		window.renderBegin();
		batchManager.render();
		console.render();
		window.renderEnd();
	}

	codex::uninitialize();
}