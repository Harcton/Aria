#include "Protocol.h"
#include <assert.h>
#include <cstring>

namespace codex
{
	namespace protocol
	{
		BufferBase::BufferBase()
			: capacity(0)
			, offset(0)
			, reversedByteOrder(false)
		{

		}

		BufferBase::~BufferBase()
		{

		}

		size_t BufferBase::getCapacity() const
		{
			return capacity;
		}

		size_t BufferBase::getWrittenSize() const
		{
			return offset;
		}

		WriteBuffer::WriteBuffer(const Endianness writeEndianness)
			: data(nullptr)
		{
			reversedByteOrder = writeEndianness == Endianness::inverted;
		}

		WriteBuffer::WriteBuffer(const Endianness writeEndianness, const size_t _capacity)
			: data(nullptr)
		{
			reversedByteOrder = writeEndianness == Endianness::inverted;
			if (_capacity > 0)
			{
				capacity = _capacity;
				data = new unsigned char[_capacity];
			}
		}

		WriteBuffer::~WriteBuffer()
		{
			delete[] data;
		}

		size_t WriteBuffer::write(const void* buffer, const size_t bytes)
		{
			if (offset + bytes > capacity && !extend(offset + bytes - capacity))
			{
				log::error("Cannot write to buffer! Cannot extend the buffer!");
				return 0;
			}

			if (reversedByteOrder)
			{//Write in reversed byte order
				const size_t endOffset = bytes - 1;
				for (size_t i = 0; i < bytes; i++)
					memcpy(&data[offset + i], &(((const unsigned char*)buffer)[endOffset - i]), 1);
			}
			else
			{//Write using the native byte order
				memcpy(&data[offset], buffer, bytes);
			}
			offset += bytes;
			return bytes;
		}

		bool WriteBuffer::extend(const size_t addedBytes)
		{
			if (addedBytes == 0)
				return true;

			unsigned char* allocation = new unsigned char[capacity + addedBytes];
			if (!allocation)
				return false;//Failed to allocate...

			//Relocate data
			if (data)
				memcpy(allocation, data, capacity);

			//Increment the capacity
			capacity += addedBytes;

			//Deallocate previous allocation
			if (data)
				delete[] data;

			//Set data to point to the new allocation
			data = allocation;

			return true;
		}

		ReadBuffer::ReadBuffer(const void* pointedMemory, const size_t length)
			: data((const unsigned char*)pointedMemory)
		{
			assert(pointedMemory);
			assert(length > 0);
			capacity = length;
		}

		ReadBuffer::~ReadBuffer()
		{
			//NOTE: do not deallocate data! data is owned by an external source!
		}

		size_t ReadBuffer::read(void* destination, const size_t bytes)
		{
			if (offset + bytes > capacity)
			{
				log::warning("Cannot read past the buffer!");
				return 0;
			}

			if (reversedByteOrder)
			{//Read in reversed byte order
				const size_t endOffset = offset + bytes - 1;
				for (size_t i = 0; i < bytes; i++)
					memcpy(&(((unsigned char*)destination)[i]), &data[endOffset - i], 1);
			}
			else
			{//Read in native byte order
				memcpy(destination, &data[offset], bytes);
			}

			offset += bytes;
			return bytes;
		}

		void ReadBuffer::translate(const int translationOffset)
		{
			offset += translationOffset;
		}

		size_t ReadBuffer::getBytesRemaining() const
		{
			return capacity - offset;
		}

		namespace aria
		{
			Serializable* Serializable::deserializeFromReadBuffer(ReadBuffer& buffer)
			{
				Type type;
				buffer.read(&type, sizeof(type));
				Serializable* serializable = nullptr;
				switch (type)
				{
				default: log::warning("codex::protocol::aria::Serializable::deserializeFromReadBuffer: invalid Type!"); return nullptr;
				case Type::Handshake: serializable = new Handshake();
				case Type::String: serializable = new String();
				case Type::GhostQuery: serializable = new GhostQuery();
				case Type::GhostOffer: serializable = new GhostOffer();
				}
				serializable->read(buffer);
				return serializable;
			}

			static const uint16_t systemEndianness = 0x00FF;
			Handshake::Handshake()
				: endianness(systemEndianness)
				, handshakeVersion(1)
				, valid(true)
			{
			}

			Endianness Handshake::getEndianness() const
			{
				return systemEndianness == endianness ? Endianness::equal : Endianness::inverted;
			}

			bool Handshake::isValid() const
			{
				return valid;
			}

			size_t Handshake::write(WriteBuffer& buffer) const
			{
				size_t offset = Serializable::write(buffer);
				offset += buffer.write(&endianness, sizeof(endianness));
				offset += buffer.write(&handshakeVersion, sizeof(handshakeVersion));
				return offset;
			}

			size_t Handshake::read(ReadBuffer& buffer)
			{//NOTE: buffer can contain invalid data! If so, set the valid boolean to false
				size_t offset = Serializable::read(buffer);
				valid = true;

				//Endianness
				if (buffer.getBytesRemaining() < sizeof(endianness))
				{
					valid = false;
					return offset;
				}
				else
					offset += buffer.read(&endianness, sizeof(endianness));
				buffer.setReversedByteOrder(getEndianness() == Endianness::inverted);
				//Handshake version
				if (buffer.getBytesRemaining() < sizeof(handshakeVersion))
				{
					valid = false;
					return offset;
				}
				else
					offset += buffer.read(&handshakeVersion, sizeof(handshakeVersion));

				return offset;
			}
		}
	}
}