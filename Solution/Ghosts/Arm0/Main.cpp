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
#include <Codex/Codex.h>
#include <Codex/SyncManager.h>
#include <Codex/Device/Servo.h>
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
	const codex::protocol::Endpoint endpoint = codex::protocol::commandLineArgumentsToEndpoint(argc, argv);
	if (endpoint == codex::protocol::Endpoint::invalid)
		return 1;
	codex::SyncManager syncManager;
	syncManager.registerType<codex::device::ServoGhost, codex::device::ServoShell>();
	syncManager.connect(endpoint);
	syncManager.initialize();

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
		syncManager.update((codex::time::TimeType)deltaTimeSystem.deltaTime.value);

		//Render
		spehs::getMainWindow()->renderBegin();
		batchManager.render();
		spehs::console::render();
		spehs::getMainWindow()->renderEnd();
	}

	spehs::uninitialize();		
	codex::uninitialize();
}