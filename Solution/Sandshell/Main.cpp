#include <Codex/Codex.h>
#include <Codex/Log.h>
#include <Codex/Device/HC_SR04.h>
#include <Codex/Device/Ublox_M8N.h>
#include <iostream>

int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);

	codex::log::info("Sandshell");
	codex::device::Ublox_M8N gps;
	gps.setPins(codex::gpio::pin_35, codex::gpio::pin_37);
	gps.start();
	while (gps.isRunning())
	{
		//Block
	}

	codex::uninitialize();
}