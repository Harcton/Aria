#pragma once
#include <stdint.h>
#include <cstring>
#include "Log.h"

namespace codex
{
	namespace protocol
	{
		typedef uint16_t OfferVersionType;

		enum class Endianness : unsigned char
		{//Byte endianness ordering
			invalid = 0,
			equal = 1,
			inverted = 2,
		};

		enum class ContactPacketType : uint16_t
		{/////////////////////////0xFF00 endianness detection
			requestOffer	= 0xFF01,
			offer			= 0xFF02,
			acceptOffer		= 0xFF03,
		};

		Endianness getEndianness(const ContactPacketType contactPacketType);

		struct RequestOffer
		{
			OfferVersionType offerVersion;//The latest offer version that the shell is capable to read
		};

		struct Offer
		{
			//Offer data
			//MTU
			//Available processing power
		};

		class BaseBuffer
		{
		public:
			BaseBuffer();
			BaseBuffer(const size_t _size);
			BaseBuffer(const void* buffer, const size_t length);
			virtual ~BaseBuffer();

			bool resize(const size_t newSize);

			size_t getSize() const;
			const unsigned char* getData() const;

		protected:
			unsigned char* data;
			size_t size;
			size_t offset;
		};

		class WriteBuffer : public BaseBuffer
		{
		public:
			WriteBuffer();
			WriteBuffer(const size_t _size);
			WriteBuffer(const void* buffer, const size_t length);
			~WriteBuffer() override;

			size_t write(const void* buffer, const size_t length);
			const unsigned char* operator[](const size_t index) const;
		};

		class ReadBuffer : public BaseBuffer
		{
		public:
			ReadBuffer();
			ReadBuffer(const size_t _size);
			ReadBuffer(const void* buffer, const size_t length);
			~ReadBuffer() override;

			size_t read(const void* buffer, const size_t length);
		};
	}
}