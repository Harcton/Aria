#include "Codex/StringOperations.h"

char hexTable[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
std::string toHexString(const uint8_t integer)
{
	std::string str(2, '0');
	str[0] = hexTable[integer >> 4];
	str[1] = hexTable[uint8_t(integer << 4) >> 4];
	return str;
}
std::string toHexString(const uint16_t integer)
{
	std::string str(4, '0');
	str[0] = hexTable[integer >> 12];
	str[1] = hexTable[uint16_t(integer << 4) >> 12];
	str[2] = hexTable[uint16_t(integer << 8) >> 12];
	str[3] = hexTable[uint16_t(integer << 12) >> 12];
	return str;
}
std::string toHexString(const uint32_t integer)
{
	std::string str(8, '0');
	str[0] = hexTable[integer >> 28];
	str[1] = hexTable[uint32_t(integer << 4) >> 28];
	str[2] = hexTable[uint32_t(integer << 8) >> 28];
	str[3] = hexTable[uint32_t(integer << 12) >> 28];
	str[4] = hexTable[uint32_t(integer << 16) >> 28];
	str[5] = hexTable[uint32_t(integer << 20) >> 28];
	str[6] = hexTable[uint32_t(integer << 24) >> 28];
	str[7] = hexTable[uint32_t(integer << 28) >> 28];
	return str;
}
std::string toHexString(const uint64_t integer)
{
	std::string str(16, '0');
	str[0] = hexTable[integer >> 60];
	str[1] = hexTable[(integer << 4) >> 60];
	str[2] = hexTable[(integer << 8) >> 60];
	str[3] = hexTable[(integer << 12) >> 60];
	str[4] = hexTable[(integer << 16) >> 60];
	str[5] = hexTable[(integer << 20) >> 60];
	str[6] = hexTable[(integer << 24) >> 60];
	str[7] = hexTable[(integer << 28) >> 60];
	str[8] = hexTable[(integer << 32) >> 60];
	str[9] = hexTable[(integer << 36) >> 60];
	str[10] = hexTable[(integer << 40) >> 60];
	str[11] = hexTable[(integer << 44) >> 60];
	str[12] = hexTable[(integer << 48) >> 60];
	str[13] = hexTable[(integer << 52) >> 60];
	str[14] = hexTable[(integer << 56) >> 60];
	str[15] = hexTable[(integer << 60) >> 60];
	return str;
}