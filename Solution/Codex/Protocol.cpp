#include <assert.h>
#include <cstring>
#include <atomic>
#include "Protocol.h"
#include "SocketTCP.h"
#include "SpehsEngine/Core/Time.h"
#include "SpehsEngine/Core/StringOperations.h"
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
				spehs::log::info("Provided command line arguments cannot be used to form an endpoint: too few arguments.");
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
				spehs::log::warning("Provided server address is invalid: " + endpoint.address);
				return Endpoint::invalid;
			}
			if (endpoint.port < 0 || endpoint.port > std::numeric_limits<uint16_t>::max())
			{
				spehs::log::warning("Provided server port is invalid: " + std::string(argv[2]));
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
		
		void WriteBuffer::reserve(const size_t capacity)
		{
			SPEHS_ASSERT(capacity >= data.capacity());
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
			SPEHS_ASSERT(pointedMemory);
			SPEHS_ASSERT(length > 0);
		}

		ReadBuffer::~ReadBuffer()
		{
			//NOTE: do not deallocate data! data is owned by an external source!
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
		void Handshake::write(WriteBuffer& buffer) const
		{
			buffer.write(currentMagic);
			buffer.write(handshakeVersion);
			buffer.write(endiannessCheckBytes);
		}

		void Handshake::read(ReadBuffer& buffer)
		{//NOTE: buffer can contain invalid data! If so, set the valid boolean to false
			size_t offset = 0;
			valid = true;

			//Magic
			if (buffer.getBytesRemaining() < sizeof(magic))
			{
				spehs::log::info("Handshake::read() invalid handshake. No bytes left to read magic.");
				valid = false;
				return;
			}
			else
				buffer.read(magic);
			if (magic != currentMagic)
			{
				spehs::log::info("Handshake::read() invalid handshake. Incompatible magic - my version: " + spehs::toHexString(currentMagic) + ", read magic: " + spehs::toHexString(magic));
				valid = false;
				return;
			}

			//Handshake version
			if (buffer.getBytesRemaining() < sizeof(handshakeVersion))
			{
				spehs::log::info("Handshake::read() invalid handshake. No bytes left to read handshake version.");
				valid = false;
				return;
			}
			else
				buffer.read((uint32_t&)handshakeVersion);
			if (handshakeVersion != currentHandshakeVersion)
			{
				spehs::log::info("Handshake::read() invalid handshake. Incompatible versions - my version: " + std::to_string(currentHandshakeVersion) + ", other version: " + std::to_string(handshakeVersion));
				valid = false;
				return;
			}
			//Endianness check bytes
			uint16_t readEndiannessCheckBytes;
			if (buffer.getBytesRemaining() < sizeof(endiannessCheckBytes))
			{
				spehs::log::info("Handshake::read() invalid handshake. No bytes left to read endianness check bytes.");
				valid = false;
				return;
			}
			else
				buffer.read(readEndiannessCheckBytes);
			if (readEndiannessCheckBytes != endiannessCheckBytes)
			{
				spehs::log::info("Handshake::read() invalid handshake. Invalid endianness check bytes.");
				valid = false;
				return;
			}
		}
	}
}