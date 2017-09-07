#include "Shell0.h"
#include <Codex/CodexTime.h>

Shell0::Shell0()
{

}

Shell0::~Shell0()
{

}

void Shell0::onStart()
{
	codex::log::info("Shell0 start");
}

void Shell0::update()
{
	codex::log::info("Shell0 update");
	codex::time::delay(codex::time::seconds(10));
}

void Shell0::onStop()
{
	codex::log::info("Shell0 stop");
}

void Shell0::receiveHandler(codex::protocol::ReadBuffer& buffer)
{
	codex::log::info("Shell0 receive handler invoked. Bytes: " + std::to_string(buffer.getCapacity()));
}
