#include "Shell0.h"
#include <Codex/CodexTime.h>
#include <Codex/CodexMath.h>
#include <Codex/GPIO.h>

Shell0::Shell0()
	: sendUpdateInterval(codex::time::seconds(1.0f / 30.0f))
	, timeSinceSendUpdate(0)
{
	dcMotorController.setPins(codex::gpio::pin_11, codex::gpio::pin_13, codex::gpio::pin_15);
	steerServo.setPin(codex::gpio::pin_3);
	steerServo.setMinAngle(codex::time::milliseconds(1.9f), -0.25f * codex::math::pi);
	steerServo.setMaxAngle(codex::time::milliseconds(0.65f), 0.25f * codex::math::pi);
	steerServo.setRotationSpeed(3.0f);
}

Shell0::~Shell0()
{

}

void Shell0::onStart()
{
	codex::log::info("Shell0 start");
}

void Shell0::update(const codex::time::TimeType deltaTime)
{
	const float deltaSeconds = float(deltaTime) / float(codex::time::conversionRate::second);
	
	//Send update
	timeSinceSendUpdate += deltaTime;
	if (timeSinceSendUpdate > sendUpdateInterval)
	{
		const codex::time::TimeType sendUpdateBeginTime = codex::time::getRunTime();
		sendUpdate();
		static const codex::time::TimeType warningThresholdTime = codex::time::seconds(0.5f);
		if (codex::time::getRunTime() - sendUpdateBeginTime >= warningThresholdTime)
			codex::log::warning("Shell sendUpdate took " + std::to_string(float(codex::time::getRunTime() - sendUpdateBeginTime) / codex::time::conversionRate::millisecond) + " ms");
	}

	static codex::time::TimeType timeSinceLastLog = 0;
	timeSinceLastLog += deltaTime;
	if (timeSinceLastLog >= codex::time::seconds(1))
	{
		timeSinceLastLog = 0;
		//codex::log::info("Runtime: " + std::to_string(codex::time::getRunTime() / codex::time::conversionRate::second) + " seconds. Latest delta time: " + std::to_string(deltaTime));
		//codex::log::info("DT: " + std::to_string(float(deltaTime) / float(codex::time::conversionRate::millisecond)) + " ms, " +
		//	std::to_string(float(timeSinceSendUpdate) / float(codex::time::conversionRate::millisecond)) + "/" +
		//	std::to_string(float(sendUpdateInterval) / float(codex::time::conversionRate::millisecond)));
	}
}

void Shell0::onStop()
{
	codex::log::info("Shell0 stop");
	//dcMotorController.stop();
	//steerServo.stop();
}

void Shell0::receiveHandler(codex::protocol::ReadBuffer& buffer)
{
	//codex::log::info("Shell0 receive handler invoked. Bytes: " + std::to_string(buffer.getCapacity()));
	codex::robot::robot0::PacketType packetType;
	buffer.read(packetType);
	switch (packetType)
	{
	default:
		codex::log::warning("Received unknown packet type: " + std::to_string((int)packetType));
		break;
	case codex::robot::robot0::PacketType::update:
		ghostNetState.read(buffer);
		dcMotorController.setStrength(ghostNetState.dcMotorStrength);
		steerServo.setTargetAngle(ghostNetState.steerAngle);
		if (ghostNetState.runSteerServo != steerServo.isRunning())
		{
			if (ghostNetState.runSteerServo)
				steerServo.start();
			else
				steerServo.stop();
		}
		if (ghostNetState.runDCMotor != dcMotorController.isRunning())
		{
			if (ghostNetState.runDCMotor)
				dcMotorController.start();
			else
				dcMotorController.stop();
		}
		break;
	}
}

void Shell0::sendUpdate()
{
	//Update shell net state
	shellNetState.dcMotorStrength = dcMotorController.getStrength();
	shellNetState.steerAngle = steerServo.getApproximatedAngle();
	shellNetState.runDCMotor = dcMotorController.isRunning();
	shellNetState.runSteerServo = steerServo.isRunning();

	//Write and send
	codex::protocol::WriteBuffer buffer;
	buffer.write(codex::robot::robot0::PacketType::update);
	shellNetState.write(buffer);
	sendPacket(buffer);

	//Reset timer
	timeSinceSendUpdate = 0;
	//codex::log::info("U");
}