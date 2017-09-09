#pragma once
#include <Codex/Actor.h>
#include <Codex/DCMotorController.h>
#include <Codex/Servo.h>
#include <Codex/Robots/Robot0.h>

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

	codex::DCMotorController dcMotorController;
	codex::Servo steerServo;
	codex::time::TimeType sendUpdateInterval;
	codex::time::TimeType timeSinceSendUpdate;

	codex::robots::robot0::GhostNetState ghostNetState;
	codex::robots::robot0::ShellNetState shellNetState;
};