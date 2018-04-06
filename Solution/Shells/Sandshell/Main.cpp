#include <boost/system/error_code.hpp>//->Works
#include "SpehsEngine/GPIO/Device/HC_SR04.h"
#include "SpehsEngine/GPIO/Device/RS232_PinReader.h"
#include "SpehsEngine/GPIO/Device/PIR_MotionSensor.h"
#include "SpehsEngine/Core/Core.h"
#include "SpehsEngine/Core/Log.h"
#include "SpehsEngine/Net/Net.h"
#include "SpehsEngine/Sync/Sync.h"
#include "SpehsEngine/GPIO/GPIO.h"
#include <glm/glm.hpp>
#include <iostream>
#include <boost/system/error_code.hpp>


int main(const int argc, const char** argv)
{
	spehs::CoreLib coreLib;
	spehs::NetLib netLib(coreLib);
	spehs::SyncLib syncLib(netLib);
	spehs::GPIOLib gpioLib(syncLib);

	spehs::log::info("Sandshell");
		
	////Motion sensor test
	//codex::device::PIR_MotionSensor motionSensor;
	//motionSensor.setPin(codex::gpio::pin_7);
	//motionSensor.start();
	//while (motionSensor.isRunning())
	//{ /* Keep running */ }
	//spehs::log::info("main end"); return 0;
	
	while (true)
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

		//std::vector<uint8_t> receiveBuffer;
		//while (reader.isRunning())
		//{
		//	reader.retrieveReceiveBuffer(receiveBuffer);
		//	//if (receiveBuffer.size() > 80)
		//	//{//Print
		//	//	std::string hexStr;
		//	//	std::string charStr;
		//	//	hexStr.reserve(3 * receiveBuffer.size());
		//	//	charStr.reserve(receiveBuffer.size());
		//	//	for (size_t i = 0; i < receiveBuffer.size(); i++)
		//	//	{
		//	//		hexStr += " " + toHexString(receiveBuffer[i]);
		//	//		charStr += receiveBuffer[i];
		//	//	}

		//	//	receiveBuffer.clear();
		//	//	//spehs::log::info(hexStr + charStr);
		//	//	reader.stop();
		//	//}
		//}
				
		std::getchar();
	}
}