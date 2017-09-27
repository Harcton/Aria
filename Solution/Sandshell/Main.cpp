#include <Codex/Codex.h>
#include <Codex/Log.h>
#include <Codex/Device/HC_SR04.h>
#include <Codex/Device/RS232_PinReader.h>
#include <iostream>


char hexTable[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
std::string toHexString(const uint8_t integer)
{
	std::string str(2, '0');
	str[0] = hexTable[integer >> 4];
	str[1] = hexTable[uint8_t(integer << 4) >> 4];
	return str;
}
std::string toHexString(const int8_t integer)
{
	std::string str(2, '0');
	str[0] = hexTable[integer >> 4];
	str[1] = hexTable[uint8_t(integer << 4) >> 4];
	return str;
}


int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);

	codex::log::info("Sandshell");

	while (true)
	{
		codex::device::RS232_PinReader reader;
		reader.setPin(codex::gpio::pin_35/*, codex::gpio::pin_37*/);
		reader.setReadInterval(codex::time::seconds(1.0f / 9600.0f));
		
		reader.setStreamBoundaryRequiredPatternRepetitionCount(10);

		/*none of the packets (in u-center) have the 8th bit enabled -> transmission unit is 7 bits wide?*/
		reader.setTransmissionUnitLength(7);

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