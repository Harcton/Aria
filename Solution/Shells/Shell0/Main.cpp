#include <boost/system/error_code.hpp>//->Works
#include <SpehsEngine/Core/Log.h>
#include <SpehsEngine/Core/Core.h>
#include "SpehsEngine/Net/Net.h"
#include "SpehsEngine/Sync/Sync.h"
#include "SpehsEngine/GPIO/GPIO.h"


#include "SpehsEngine/GPIO/Device/RS232_PinReader.h"
/*
	Arguments:
	0: path
	1: connect address
	2. connect port

	or

	0: path
	1. receive port
*/
int main(const int argc, const char** argv)
{
	spehs::CoreLib coreLib;
	spehs::NetLib netLib(coreLib);
	spehs::SyncLib syncLib(netLib);
	spehs::GPIOLib gpioLib(syncLib);

	bool keepRunning = true;
	while (keepRunning)
	{

		spehs::device::RS232_PinReader reader;
		reader.setPin(spehs::gpio::pin_35/*, codex::gpio::pin_37*/);
		reader.setReadInterval(spehs::time::fromSeconds(1.0f / 9600.0f));
		//reader.setReadInterval(spehs::time::fromSeconds(1.0f / 4800.0f));

		reader.setStreamBoundaryRequiredPatternRepetitionCount(10);
		reader.setStopBitCount(1);

		/*none of the packets (in u-center) have the 8th bit enabled -> transmission unit is 7 bits wide?*/
		reader.setTransmissionUnitLength(8);

		reader.disableParityBitCheck();
		//reader.enableParityBitCheck();

		reader.start();
	}

	return 0;
}


///*
////TEST BASE
//
//std::atomic<int> strength(0);
//std::atomic<bool> keepRunningInput(false);
//void runInput()
//{
//keepRunningInput = true;
//while (keepRunningInput)
//{
//spehs::log::info("Input strength:\n>");
//int val;
//std::cin >> val;
//strength = val;
//}
//}
//
////Main
//std::thread inputThread(runInput);
//codex::gpio::Pin pin = codex::gpio::pin_11;
//codex::gpio::setPinAsOutput(pin);
//while (strength >= 0)
//{
//spehs::time::Time interval = spehs::time::fromMilliseconds(1);
//spehs::time::delay(interval);
//}
//inputThread.join();
//
//*/