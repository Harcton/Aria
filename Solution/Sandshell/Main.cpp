#include <Codex/Codex.h>
#include <Codex/Log.h>
#include <Codex/Device/HC_SR04.h>
#include <iostream>

int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);

	codex::log::info("Sandshell");
	codex::device::HC_SR04 sensor;

	codex::uninitialize();
}