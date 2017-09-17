#pragma once
#include <Codex/Actor.h>
#include <Codex/Device/DCMotorController.h>
#include <Codex/Device/Servo.h>
#include <Codex/Device/HC_SR04.h>
#include <Codex/Robot/Robot0.h>

class Shell0 : public codex::Actor
{
public:
	Shell0();
	~Shell0() override;
	
private:

	void onStart() override;
	void update(const codex::time::TimeType deltaTime) override;
	void onStop() override;

	void receiveHandler(codex::protocol::ReadBuffer& buffer) override;

	void sendUpdate();

	codex::device::DCMotorController dcMotorController;
	codex::device::Servo steerServo;
	codex::device::HC_SR04 distanceSensor;
	codex::time::TimeType sendUpdateInterval;
	codex::time::TimeType timeSinceSendUpdate;

	codex::robot::robot0::GhostNetState ghostNetState;
	codex::robot::robot0::ShellNetState shellNetState;
};