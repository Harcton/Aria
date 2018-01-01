#include <Codex/Codex.h>
#include <Codex/Log.h>
#include <Codex/Device/HC_SR04.h>
#include <Codex/Device/RS232_PinReader.h>
#include <Codex/Device/PIR_MotionSensor.h>
#include <glm/glm.hpp>
#include <iostream>
#include <boost/system/error_code.hpp>


int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);

	codex::log::info("Sandshell");
		
	////Motion sensor test
	//codex::device::PIR_MotionSensor motionSensor;
	//motionSensor.setPin(codex::gpio::pin_7);
	//motionSensor.start();
	//while (motionSensor.isRunning())
	//{ /* Keep running */ }
	//codex::log::info("main end"); return 0;
	

	while (true)
	{
		codex::device::RS232_PinReader reader;
		reader.setPin(codex::gpio::pin_35/*, codex::gpio::pin_37*/);
		reader.setReadInterval(codex::time::seconds(1.0f / 9600.0f));
		//reader.setReadInterval(codex::time::seconds(1.0f / 4800.0f));
		
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
		//	//	//codex::log::info(hexStr + charStr);
		//	//	reader.stop();
		//	//}
		//}
				
		std::getchar();
	}

	codex::uninitialize();
}