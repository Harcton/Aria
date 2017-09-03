#include <assert.h>
#include <cstring>
#include <atomic>
#include "Protocol.h"
#include "SocketTCP.h"
#include "CodexTime.h"
#ifdef _WIN32
#include <Windows.h>//NOTE: must be included after boost asio stuff...
#endif

namespace codex
{
	namespace protocol
	{
		const PortType defaultAriaPort = 49842;
		const uint16_t localEndiannessBytePair = 0x00FF;
		const uint16_t invertedEndiannessBytePair = 0xFF00;



		BufferBase::BufferBase(Endianness _endianness)
			: capacity(0)
			, offset(0)
			, endianness(_endianness)
		{
			if (endianness == Endianness::undefined)
				log::error("BufferBase was constructed with undefined endianness!");
		}

		BufferBase::~BufferBase()
		{

		}

		size_t BufferBase::getCapacity() const
		{
			return capacity;
		}

		size_t BufferBase::getOffset() const
		{
			return offset;
		}

		Endianness BufferBase::getEndianness() const
		{
			return endianness;
		}



#ifdef SHELL_CODEX
		//The shell codex should never be obligated to write in specified byte order
		WriteBuffer::WriteBuffer()
			: BufferBase(Endianness::local)
			, data(nullptr)
		{

		}
#else
		WriteBuffer::WriteBuffer(const Endianness _endianness)
			: BufferBase(_endianness)
			, data(nullptr)
		{

		}
#endif

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

			if (endianness == Endianness::inverted)
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
		size_t WriteBuffer::write(const uint8_t value)
		{
			return write(&value, sizeof(value));
		}
		size_t WriteBuffer::write(const int8_t value)
		{
			return write(&value, sizeof(value));
		}
		size_t WriteBuffer::write(const uint16_t value)
		{
			return write(&value, sizeof(value));
		}
		size_t WriteBuffer::write(const int16_t value)
		{
			return write(&value, sizeof(value));
		}
		size_t WriteBuffer::write(const uint32_t value)
		{
			return write(&value, sizeof(value));
		}
		size_t WriteBuffer::write(const int32_t value)
		{
			return write(&value, sizeof(value));
		}
		size_t WriteBuffer::write(const uint64_t value)
		{
			return write(&value, sizeof(value));
		}
		size_t WriteBuffer::write(const int64_t value)
		{
			return write(&value, sizeof(value));
		}
		size_t WriteBuffer::write(const bool value)
		{
			return write(&value, sizeof(value));
		}
		size_t WriteBuffer::write(const std::string& value)
		{
			const uint32_t length = value.size();
			write(&length, sizeof(length));
			if (length == 0)
				return sizeof(length);
			return sizeof(length) + write(&value[0], length);
		}
		size_t WriteBuffer::write(const PacketType value)
		{
			return write(&value, sizeof(value));
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


		ReadBuffer::ReadBuffer(const void* pointedMemory, const size_t length, const Endianness _endianness)
			: BufferBase(_endianness)
			, data((const unsigned char*)pointedMemory)
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

			if (endianness == Endianness::inverted)
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

		size_t ReadBuffer::read(uint8_t& value)
		{
			return read(&value, sizeof(value));
		}

		size_t ReadBuffer::read(int8_t& value)
		{
			return read(&value, sizeof(value));
		}

		size_t ReadBuffer::read(uint16_t& value)
		{
			return read(&value, sizeof(value));
		}

		size_t ReadBuffer::read(int16_t& value)
		{
			return read(&value, sizeof(value));
		}

		size_t ReadBuffer::read(uint32_t& value)
		{
			return read(&value, sizeof(value));
		}

		size_t ReadBuffer::read(int32_t& value)
		{
			return read(&value, sizeof(value));
		}

		size_t ReadBuffer::read(uint64_t& value)
		{
			return read(&value, sizeof(value));
		}

		size_t ReadBuffer::read(int64_t& value)
		{
			return read(&value, sizeof(value));
		}

		size_t ReadBuffer::read(bool& value)
		{
			return read(&value, sizeof(value));
		}
		size_t ReadBuffer::read(std::string& value)
		{
			uint32_t length;
			read(length);
			value.resize(length);
			if (length > 0)
				read(&value[0], length);
			return sizeof(length) + length;
		}
		size_t ReadBuffer::read(PacketType& value)
		{
			return read(&value, sizeof(value));
		}

		void ReadBuffer::translate(const int translationOffset)
		{
			offset += translationOffset;
		}

		size_t ReadBuffer::getBytesRemaining() const
		{
			return capacity - offset;
		}
		
		const VersionType currentHandshakeVersion = 1;
		Handshake::Handshake()
			: endiannessBytePair(localEndiannessBytePair)
			, handshakeVersion(currentHandshakeVersion)
			, codexType(codex::codexType)
			, valid(true)
		{
		}

		Endianness Handshake::getEndianness() const
		{
			if (endiannessBytePair == localEndiannessBytePair)
				return Endianness::local;
			else if (endiannessBytePair == invertedEndiannessBytePair)
				return Endianness::inverted;
			else
				return Endianness::undefined;
		}

		CodexType Handshake::getCodexType() const
		{
			if (codexType == CodexType::ghost || codexType == CodexType::shell)
				return codexType;
			else
				return CodexType::invalid;
		}

		bool Handshake::isValid() const
		{
			return valid;
		}

		size_t Handshake::write(WriteBuffer& buffer) const
		{
			size_t offset = 0;
			offset += buffer.write(&endiannessBytePair, sizeof(endiannessBytePair));
			offset += buffer.write(&handshakeVersion, sizeof(handshakeVersion));
			offset += buffer.write(&codexType, sizeof(codexType));
			return offset;
		}

		size_t Handshake::read(ReadBuffer& buffer)
		{//NOTE: buffer can contain invalid data! If so, set the valid boolean to false
			size_t offset = 0;
			valid = true;

			//Endianness
			if (buffer.getBytesRemaining() < sizeof(endiannessBytePair))
			{
				log::info("Handshake::read() invalid handshake. No bytes left to read endianness byte pair.");
				valid = false;
				handshakeVersion = 0;
				endiannessBytePair = 0;
				codexType = CodexType::invalid;
				return offset;
			}
			else
				offset += buffer.read(&endiannessBytePair, sizeof(endiannessBytePair));
			buffer.endianness = getEndianness();//Update buffer read byte order
			if (getEndianness() == Endianness::undefined)
			{
				log::info("Handshake::read() invalid handshake. Endianness byte pair is invalid.");
				valid = false;
				return offset;
			}
			//Handshake version
			if (buffer.getBytesRemaining() < sizeof(handshakeVersion))
			{
				log::info("Handshake::read() invalid handshake. No bytes left to read handshake version.");
				valid = false;
				return offset;
			}
			else
				offset += buffer.read(&handshakeVersion, sizeof(handshakeVersion));
			if (handshakeVersion != currentHandshakeVersion)
			{
				log::info("Handshake::read() invalid handshake. Incompatible versions - my version: " + std::to_string(currentHandshakeVersion) + ", other version: " + std::to_string(handshakeVersion));
				valid = false;
				return offset;
			}
			//Codex implementation
			if (buffer.getBytesRemaining() < sizeof(codexType))
			{
				log::info("Handshake::read() invalid handshake. No bytes left to read codex type.");
				valid = false;
				return offset;
			}
			else
				offset += buffer.read(&codexType, sizeof(codexType));
			if (getCodexType() == CodexType::invalid)
			{
				log::info("Handshake::read() invalid handshake. Invalid codex type: " + std::to_string((int)codexType));
				valid = false;
				return offset;
			}

			return offset;
		}
	}
}