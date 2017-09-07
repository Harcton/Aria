#pragma once
#include <Codex/Ghost.h>
#include <SpehsEngine/Camera2D.h>
#include <SpehsEngine/BatchManager.h>

class Ghost0 : public codex::Ghost
{
public:
	Ghost0();
	~Ghost0();

	void onStart() override;
	void update() override;
	void onStop() override;

	void receiveHandler(codex::protocol::ReadBuffer& buffer) override;

private:
	spehs::Camera2D camera;
	spehs::BatchManager batchManager;

};

