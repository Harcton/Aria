#pragma once
#include <Codex/Shell.h>

class Shell0 : public codex::Shell
{
public:
	Shell0();
	~Shell0();

	void onStart() override;
	void update() override;
	void onStop() override;

	void receiveHandler(codex::protocol::ReadBuffer& buffer) override;

private:

};

