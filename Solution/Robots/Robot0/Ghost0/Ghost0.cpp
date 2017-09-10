#include "Ghost0.h"
#include <Codex/CodexMath.h>
#include <SpehsEngine/ApplicationData.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/AudioEngine.h>
#include <SpehsEngine/SpehsEngine.h>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/Time.h>

Ghost0::Ghost0()
	: batchManager(&camera, "Ghost0")
	, sendUpdateInterval(codex::time::seconds(1.0f / 30.0f))
	, lastUpdateTime(codex::time::getRunTime())
	, timeSinceSendUpdate(0)
	, steerServoAngleArrow()
	, shellDCMotorStrengthArrow()
	, updatesReceived(0)
	, updatesSent(0)
{
	batchManager.beginSection();
	steerServoAngleArrow = new spehs::Arrow();
	shellDCMotorStrengthArrow = new spehs::Arrow();
	debugText = spehs::Text::create();
}

Ghost0::~Ghost0()
{
	delete steerServoAngleArrow;
	delete shellDCMotorStrengthArrow;
	debugText->destroy();
	batchManager.endSection();
}

void Ghost0::onStart()
{
	spehs::initialize("Ghost0");
	steerServoAngleArrow->setCameraMatrixState(false);
	shellDCMotorStrengthArrow->setCameraMatrixState(false);
	steerServoAngleArrow->setArrowPointerSize(spehs::vec2(15.0f, 10.0f));
	shellDCMotorStrengthArrow->setArrowPointerSize(spehs::vec2(10.0f, 15.0f));
	debugText->setFont("Fonts/Anonymous.ttf", 18);
	debugText->setPosition(20, 20);
}

void Ghost0::update(const codex::time::TimeType deltaTime)
{
	//Time
	const codex::time::TimeType updateTime = codex::time::getRunTime();
	const float deltaSeconds = float(updateTime - lastUpdateTime) / float(codex::time::conversionRate::second);
	lastUpdateTime = updateTime;
	
	//Update
	spehs::time::update();
	inputManager->update();
	spehs::audio::AudioEngine::update();
	spehs::console::update();
	if (inputManager->isQuitRequested() || inputManager->isKeyPressed(KEYBOARD_ESCAPE))
		stop();
	debugText->setString(
		"DT: " + std::to_string(float(deltaTime) / float(codex::time::conversionRate::millisecond)) + " ms" +
		"\nUpdates sent: " + std::to_string(updatesSent) +
		"\nUpdates received: " + std::to_string(updatesReceived) +
		"\nSteer angle: " + std::to_string(shellNetState.steerAngle) +
		"\nDC motor controller strength: " + std::to_string(shellNetState.dcMotorStrength)
	);
	if (inputManager->isKeyPressed(KEYBOARD_1))
		ghostNetState.runDCMotor = !ghostNetState.runDCMotor;
	if (inputManager->isKeyPressed(KEYBOARD_2))
		ghostNetState.runSteerServo = !ghostNetState.runSteerServo;
	
	//DC motor arrow
	if (shellNetState.runDCMotor)
		shellDCMotorStrengthArrow->setColor(spehs::Color(0, 255, 0));
	else
		shellDCMotorStrengthArrow->setColor(spehs::Color(255, 0, 0));
	shellDCMotorStrengthArrow->setPosition(
		spehs::vec2(spehs::ApplicationData::getWindowWidthHalf(), spehs::ApplicationData::getWindowHeightHalf()),
		spehs::vec2(spehs::ApplicationData::getWindowWidthHalf(), spehs::ApplicationData::getWindowHeightHalf() * (1.0f + shellNetState.dcMotorStrength)));
	
	//Steer servo arrow
	if (shellNetState.runSteerServo)
		steerServoAngleArrow->setColor(spehs::Color(0, 255, 0));
	else
		steerServoAngleArrow->setColor(spehs::Color(255, 0, 0));
	steerServoAngleArrow->setPosition(
		spehs::vec2(spehs::ApplicationData::getWindowWidthHalf(), spehs::ApplicationData::getWindowHeightHalf()),
		spehs::vec2(
			spehs::ApplicationData::getWindowWidthHalf() + spehs::ApplicationData::getWindowWidthHalf() * cosf(shellNetState.steerAngle + codex::math::pi * 0.5f),
			spehs::ApplicationData::getWindowHeightHalf() + spehs::ApplicationData::getWindowHeightHalf() * sinf(shellNetState.steerAngle + codex::math::pi * 0.5f)));

	//Render
	spehs::getMainWindow()->renderBegin();
	batchManager.render();
	spehs::console::render();
	spehs::getMainWindow()->renderEnd();

	//Send update
	timeSinceSendUpdate += codex::time::seconds(deltaSeconds);
	if (timeSinceSendUpdate > sendUpdateInterval)
		sendUpdate();
}

void Ghost0::onStop()
{
	spehs::uninitialize();
}

void Ghost0::receiveHandler(codex::protocol::ReadBuffer& buffer)
{
	//codex::log::info("Ghost0 receive handler invoked. Bytes: " + std::to_string(buffer.getCapacity()));
	codex::robots::robot0::PacketType packetType;
	buffer.read(packetType);
	switch (packetType)
	{
	default:
		codex::log::warning("Unknown packet type received.");
		break;
	case codex::robots::robot0::PacketType::update:
		updatesReceived++;
		shellNetState.read(buffer);
		break;
	}
}

void Ghost0::sendUpdate()
{
	//Update ghost net state
	ghostNetState.dcMotorStrength = 0.0f;
	ghostNetState.steerAngle = 0.0f;
	if (inputManager->joysticks.size() > 0)
	{//Use joystick
		ghostNetState.dcMotorStrength = inputManager->joysticks[0]->getAxisState(1);
		ghostNetState.steerAngle = inputManager->joysticks[0]->getAxisState(0);
	}
	else
	{//Use keyboard
		if (inputManager->isKeyDown(KEYBOARD_A))
			ghostNetState.steerAngle += codex::math::pi * 0.25f;
		if (inputManager->isKeyDown(KEYBOARD_D))
			ghostNetState.steerAngle -= codex::math::pi * 0.25f;
		if (inputManager->isKeyDown(KEYBOARD_W))
			ghostNetState.steerAngle += 1.0f;
		if (inputManager->isKeyDown(KEYBOARD_S))
			ghostNetState.steerAngle -= 1.0f;
	}

	//Write and send
	codex::protocol::WriteBuffer buffer;
	buffer.write(codex::robots::robot0::PacketType::update);
	ghostNetState.write(buffer);
	sendPacket(buffer);
	updatesSent++;

	//Reset timer
	timeSinceSendUpdate = 0;
}