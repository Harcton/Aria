#pragma once
#include <Codex/Actor.h>
#include <SpehsEngine/Camera2D.h>
#include <SpehsEngine/BatchManager.h>
#include <Codex/Robot/Robot0.h>
#include <SpehsEngine/Arrow.h>
#include <SpehsEngine/Text.h>


class Ghost0 : public codex::Actor
{
public:
	Ghost0();
	~Ghost0();

	void onStart() override;
	void update(const codex::time::TimeType deltaTime) override;
	void onStop() override;

	void receiveHandler(codex::protocol::ReadBuffer& buffer) override;

	void sendUpdate();

private:
	spehs::Camera2D camera;
	spehs::BatchManager batchManager;

	codex::time::TimeType sendUpdateInterval;
	codex::time::TimeType lastUpdateTime;
	codex::time::TimeType timeSinceSendUpdate;

	codex::robot::robot0::ShellNetState shellNetState;
	codex::robot::robot0::GhostNetState ghostNetState;
	int updatesReceived;
	int updatesSent;

	//Spehs visualization
	spehs::Arrow* steerServoAngleArrow;
	spehs::Arrow* shellDCMotorStrengthArrow;
	spehs::Text* debugText;

};

