#pragma once
#include <Codex/Actor.h>
#include <Codex/Robot/Robot0.h>
#include <SpehsEngine/Rendering/Camera2D.h>
#include <SpehsEngine/Rendering/BatchManager.h>
#include <SpehsEngine/Rendering/Arrow.h>
#include <SpehsEngine/Rendering/Text.h>
#include <SpehsEngine/Core/Time.h>

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
	spehs::time::DeltaTimeSystem deltaTimeSystem;

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

