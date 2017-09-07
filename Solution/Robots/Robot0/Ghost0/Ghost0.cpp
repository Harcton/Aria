#include "Ghost0.h"
#include <SpehsEngine/SpehsEngine.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/AudioEngine.h>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/Time.h>

Ghost0::Ghost0()
	: batchManager(&camera, "Ghost0")
{
}

Ghost0::~Ghost0()
{
}

void Ghost0::onStart()
{
	spehs::initialize("Ghost0");
}

void Ghost0::update()
{
	//Update
	spehs::time::update();
	inputManager->update();
	spehs::audio::AudioEngine::update();
	spehs::console::update();
	if (inputManager->isQuitRequested() || inputManager->isKeyPressed(KEYBOARD_ESCAPE))
		stop();

	//Render
	spehs::getMainWindow()->renderBegin();
	batchManager.render();
	spehs::console::render();
	spehs::getMainWindow()->renderEnd();
}

void Ghost0::onStop()
{
	spehs::uninitialize();
}

void Ghost0::receiveHandler(codex::protocol::ReadBuffer& buffer)
{
	codex::log::info("Ghost0 receive handler invoked. Bytes: " + std::to_string(buffer.getCapacity()));
}
