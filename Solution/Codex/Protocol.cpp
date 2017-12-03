#include <assert.h>
#include <cstring>
#include <atomic>
#include "Protocol.h"
#include "SocketTCP.h"
#include "CodexTime.h"
#include "CodexAssert.h"
#include "StringOperations.h"
#ifdef _WIN32
#include <Windows.h>//NOTE: must be included after boost asio stuff...
#endif

namespace codex
{
	namespace protocol
	{
		std::string getEndiannessAsString(const Endianness endianness)
		{
			switch (endianness)
			{
			case Endianness::big: return "big";
			case Endianness::little: return "little";
			default: return "unknown";
			}
		}

		enum
		{
			O32_LITTLE_ENDIAN = 0x03020100ul,
			O32_BIG_ENDIAN = 0x00010203ul,
			O32_PDP_ENDIAN = 0x01000302ul
		};
		static const union { unsigned char bytes[4]; uint32_t value; } o32_host_order = { { 0, 1, 2, 3 } };

		const PortType defaultAriaPort = 49842;
		const Endianness networkByteOrder = Endianness::little;
		const Endianness hostByteOrder = o32_host_order.value == O32_BIG_ENDIAN ? Endianness::big : (o32_host_order.value == O32_LITTLE_ENDIAN ? Endianness::little : Endianness::unknown);		
		const Endpoint Endpoint::invalid("0.0.0.0", 0);

		Endpoint commandLineArgumentsToEndpoint(const int argc, const char** argv)
		{
			if (argc < 3)
			{
				log::info("Provided command line arguments cannot be used to form an endpoint: too few arguments.");
				return Endpoint::invalid;
			}

			const protocol::Endpoint endpoint(argv[1], std::atoi(argv[2]));

			//Some endpoint validation...
			int periodCount = 0;
			bool invalidCharacters = false;
			for (size_t i = 0; i < endpoint.address.size(); i++)
			{
				if (endpoint.address[i] == '.')
					periodCount++;
				else if (endpoint.address[i] < 48 || endpoint.address[i] > 57)
					invalidCharacters = true;
			}
			if (periodCount != 3 || invalidCharacters || endpoint.address.size() > 15)
			{
				codex::log::warning("Provided server address is invalid: " + endpoint.address);
				return Endpoint::invalid;
			}
			if (endpoint.port < 0 || endpoint.port > std::numeric_limits<uint16_t>::max())
			{
				codex::log::warning("Provided server port is invalid: " + std::string(argv[2]));
				return Endpoint::invalid;
			}

			return endpoint;
		}

		BufferBase::BufferBase()
			: offset(0)
		{

		}

		BufferBase::~BufferBase()
		{

		}

		size_t BufferBase::getOffset() const
		{
			return offset;
		}
		
		WriteBuffer::WriteBuffer()
		{

		}

		WriteBuffer::~WriteBuffer()
		{

		}

		size_t WriteBuffer::write(const void* buffer, const size_t bytes)
		{
			data.resize(data.size() + bytes);

			if (hostByteOrder == networkByteOrder)
			{//Write in native order
				memcpy(&data[offset], buffer, bytes);
				offset += bytes;
			}
			else
			{//Write in reversed order
				size_t endOffset = bytes;
				for (size_t i = 0; i < bytes; i++)
				{
					data[offset++] = ((const unsigned char*)buffer)[--endOffset];
				}
			}

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

		size_t WriteBuffer::write(const float value)
		{
			return write(&value, sizeof(value));
		}

		size_t WriteBuffer::write(const double value)
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

		void WriteBuffer::reserve(const size_t capacity)
		{
			CODEX_ASSERT(capacity >= data.capacity());
			data.reserve(capacity);
		}

		bool WriteBuffer::extend(const size_t addedBytes)
		{
			if (addedBytes == 0)
				return true;

			data.reserve(getCapacity() + addedBytes);

			return true;
		}
		
		ReadBuffer::ReadBuffer(const void* pointedMemory, const size_t length)
			: data((const unsigned char*)pointedMemory)
			, capacity(length)
		{
			assert(pointedMemory);
			assert(length > 0);
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

			if (hostByteOrder == networkByteOrder)
			{//Read in native byte order
				memcpy(destination, &data[offset], bytes);
			}
			else
			{//Read in reversed byte order
				size_t readOffset = offset + bytes;
				for (size_t i = 0; i < bytes; i++)
				{
					((unsigned char*)destination)[i] = data[--readOffset];
				}
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

		size_t ReadBuffer::read(float& value)
		{
			return read(&value, sizeof(value));
		}

		size_t ReadBuffer::read(double& value)
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

		const uint64_t currentMagic = 0xC0DEC5F070C01001;//Mmmm...magic
		const VersionType currentHandshakeVersion = 1;
		Handshake::Handshake()
			: magic(0xC0DEC5F070C01001)
			, handshakeVersion(currentHandshakeVersion)
			, valid(true)
		{
		}

		bool Handshake::isValid() const
		{
			return valid;
		}

		const uint16_t endiannessCheckBytes = 0xACDC;
		size_t Handshake::write(WriteBuffer& buffer) const
		{
			size_t offset = 0;
			offset += buffer.write(currentMagic);
			offset += buffer.write(handshakeVersion);
			offset += buffer.write(endiannessCheckBytes);
			return offset;
		}

		size_t Handshake::read(ReadBuffer& buffer)
		{//NOTE: buffer can contain invalid data! If so, set the valid boolean to false
			size_t offset = 0;
			valid = true;

			//Magic
			if (buffer.getBytesRemaining() < sizeof(magic))
			{
				log::info("Handshake::read() invalid handshake. No bytes left to read magic.");
				valid = false;
				return offset;
			}
			else
				offset += buffer.read(magic);
			if (magic != currentMagic)
			{
				log::info("Handshake::read() invalid handshake. Incompatible magic - my version: " + toHexString(currentMagic) + ", read magic: " + toHexString(magic));
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
				offset += buffer.read((uint32_t&)handshakeVersion);
			if (handshakeVersion != currentHandshakeVersion)
			{
				log::info("Handshake::read() invalid handshake. Incompatible versions - my version: " + std::to_string(currentHandshakeVersion) + ", other version: " + std::to_string(handshakeVersion));
				valid = false;
				return offset;
			}
			//Endianness check bytes
			uint16_t readEndiannessCheckBytes;
			if (buffer.getBytesRemaining() < sizeof(endiannessCheckBytes))
			{
				log::info("Handshake::read() invalid handshake. No bytes left to read endianness check bytes.");
				valid = false;
				return offset;
			}
			else
				offset += buffer.read(readEndiannessCheckBytes);
			if (readEndiannessCheckBytes != endiannessCheckBytes)
			{
				log::info("Handshake::read() invalid handshake. Invalid endianness check bytes.");
				valid = false;
				return offset;
			}

			return offset;
		}
	}
}