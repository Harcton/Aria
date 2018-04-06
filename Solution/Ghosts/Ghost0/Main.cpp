#include "SpehsEngine/Core/Core.h"
#include "SpehsEngine/Net/Net.h"
#include "SpehsEngine/Sync/Sync.h"
#include "SpehsEngine/GPIO/GPIO.h"


int main(const int argc, const char** argv)
{
	spehs::CoreLib coreLib;
	spehs::NetLib netLib(coreLib);
	spehs::SyncLib syncLib(netLib);
	spehs::GPIOLib gpioLib(syncLib);
}