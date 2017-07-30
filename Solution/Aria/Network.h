#pragma once
#include <stdint.h>

namespace packet
{
	enum packet : uint16_t
	{/////////////////////////0xFF00 endianness detection
		requestOffer		= 0xFF01,
		offer				= 0xFF02,
		acceptOffer			= 0xFF03,
	};

	struct RequestOffer
	{
		uint16_t offerVersion;//The latest offer version that the shell is capable to read
	};

	struct Offer
	{
		//Offer data
		//MTU
		//Available processing power
	};
}