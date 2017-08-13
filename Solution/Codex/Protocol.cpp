#include "Protocol.h"

namespace codex
{
	namespace protocol
	{
		Endianness getEndianness(const ContactPacketType contactPacketType)
		{//Detects endianness based on the sent contact packet
			const uint8_t leastSignificantByteBits = (uint16_t)contactPacketType;
			if (leastSignificantByteBits == 0xFF)
				return Endianness::inverted;
			const uint8_t mostSignificantByteBits = ((uint16_t)contactPacketType >> 8);
			if (mostSignificantByteBits == 0xFF)
				return Endianness::equal;
			return Endianness::invalid;
		}


		BaseBuffer::BaseBuffer()
			: data(nullptr)
			, size(0)
			, offset(0)
		{

		}
		BaseBuffer::BaseBuffer(const size_t _size)
			: size(_size)
			, data(new unsigned char[_size])
		{

		}
		BaseBuffer::BaseBuffer(const void* buffer, const size_t length)
			: size(length)
			, data(new unsigned char[length])
		{
			memcpy(data, buffer, length);
		}
		BaseBuffer::~BaseBuffer()
		{
			delete[] data;
		}

		bool BaseBuffer::resize(const size_t newSize)
		{
			if (data)
				delete[] data;
			data = new unsigned char[newSize];
			if (data)
			{//Allocation successful
				size = newSize;
				return true;
			}
			else
			{//Failed to allocate!
				size = 0;
				return false;
			}
		}

		size_t BaseBuffer::getSize() const
		{
			return size;
		}
		const unsigned char* BaseBuffer::getData() const
		{
			return data;
		}


		WriteBuffer::WriteBuffer()
		{

		}

		WriteBuffer::WriteBuffer(const size_t _size)
			: BaseBuffer(_size)
		{

		}

		WriteBuffer::WriteBuffer(const void* buffer, const size_t length)
			: BaseBuffer(buffer, length)
		{

		}

		WriteBuffer::~WriteBuffer()
		{

		}

		size_t WriteBuffer::write(const void* buffer, const size_t length)
		{
			if (offset + length > size)
			{
				log::error("Cannot write past the buffer!");
				return 0;
			}
			memcpy(&data[offset], buffer, length);
			offset += length;
			return length;
		}

		const unsigned char* WriteBuffer::operator[](const size_t index) const
		{
			return &data[index];
		}



		ReadBuffer::ReadBuffer()
		{

		}

		ReadBuffer::ReadBuffer(const size_t _size)
			: BaseBuffer(_size)
		{

		}

		ReadBuffer::ReadBuffer(const void* buffer, const size_t length)
			: BaseBuffer(buffer, length)
		{

		}

		ReadBuffer::~ReadBuffer()
		{
		
		}

		size_t ReadBuffer::read(const void* buffer, const size_t length)
		{
			if (offset + length > size)
			{
				log::error("Cannot write past the buffer!");
				return 0;
			}
			memcpy(&data[offset], buffer, length);
			offset += length;
			return length;
		}
	}
}