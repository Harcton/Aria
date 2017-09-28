#include "Codex/StringOperations.h"

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