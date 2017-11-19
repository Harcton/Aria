#pragma once
#include <stdint.h>
#include <cstring>
#include <assert.h>
#include "Log.h"
#include "Codex.h"

namespace codex
{
	class SocketTCP;
	namespace protocol
	{
		class WriteBuffer;
		class ReadBuffer;
		class Handshake;
		typedef uint16_t OfferVersionType;
		typedef uint32_t VersionType;
		typedef uint16_t PortType;
		typedef std::string AddressType;

		extern const PortType defaultAriaPort;
		
		/* Byte endianness ordering */
		enum class Endianness : uint8_t
		{
			unknown = 0,
			little = 1,
			big = 2,
		};
		extern const Endianness hostByteOrder;
		extern const Endianness networkByteOrder;
		extern std::string getEndiannessAsString(const Endianness endianness);

		/* Specify a reason for disconnection. */
		enum class DisconnectType : uint8_t
		{
			unspecified,					//No specified reason for disconnecting
			doNotSendDisconnectPacket,		//Used from disconnect receive handler
			error,							//The program had to terminate due to an error
			socketDestructor,				//Disconnect called from the socket's destructor
			switchEndpoint,					//The endpoint is switching endpoints
			programExit,					//Used when disconnecting the socket after running the shell/ghost main program
			timeout,						//No packets received for a 
		};

		/* Codex sockets add a packet type header into each packet. Packet type determines the contents of the packet. Currently a packet is solely limited to a single packet type. */
		enum class PacketType : uint8_t
		{
			invalid = 0,					//Not in use
			handshake,						//Packet contains codex defined handshake data
			disconnect,						//The remote end sends a packet to indicate disconnection
			syncEntryCreate,
			syncEntryUpdate,
			syncEntryRemove,

			undefined = 255,				//Sent data is user defined data, which is processed by a user defined receive handler.			
		};

		/*
			Currently codex uses the standard IPv4 endpoints that consist of a 32 bit address (presented in a string form) and a 16 bit port number.
		*/
		struct Endpoint
		{
			Endpoint(const AddressType _address, const PortType _port) : address(_address), port(_port) {}
			AddressType address;
			PortType port;
		};

		/*
			An abstract base class for read/write buffers.
			Buffer read/write handles endianness conversion.
		*/
		class BufferBase
		{
		public:
			friend class Handshake;
		public:
			BufferBase();
			virtual ~BufferBase() = 0;
			
			/* Returns the total capacity of the buffer. */
			size_t getCapacity() const;

			/* Returns the number of bytes written/read using the write/read method. */
			size_t getOffset() const;

		protected:
			size_t capacity;
			size_t offset;
		};

		/*
			Write buffers can only be extended, but never contracted.
			Owns the underlying data.
		*/
		class WriteBuffer : public BufferBase
		{
		public:
			WriteBuffer();
			~WriteBuffer() override;
			
			/* Extends the buffer by increasing its size, without affecting the current offset. */
			bool extend(const size_t addedBytes);

			size_t write(const uint8_t value);
			size_t write(const int8_t value);
			size_t write(const uint16_t value);
			size_t write(const int16_t value);
			size_t write(const uint32_t value);
			size_t write(const int32_t value);
			size_t write(const uint64_t value);
			size_t write(const int64_t value);
			size_t write(const float value);
			size_t write(const double value);
			size_t write(const bool value);
			size_t write(const std::string& value);
			size_t write(const PacketType value);

			const unsigned char* operator[](const size_t index) const { return &data[index]; }

		private:
			/* Writes to data*/
			size_t write(const void* buffer, const size_t length);

			unsigned char* data;
		};

		/*
			Does NOT own the underlying data!
		*/
		class ReadBuffer : public BufferBase
		{
		public:
			ReadBuffer(const void* pointedMemory, const size_t length);
			~ReadBuffer() override;

			size_t read(uint8_t& value);
			size_t read(int8_t& value);
			size_t read(uint16_t& value);
			size_t read(int16_t& value);
			size_t read(uint32_t& value);
			size_t read(int32_t& value);
			size_t read(uint64_t& value);
			size_t read(int64_t& value);
			size_t read(float& value);
			size_t read(double& value);
			size_t read(bool& value);
			size_t read(std::string& value);
			size_t read(PacketType& value);

			/* Translates offset by a set amount of bytes. */
			void translate(const int translationOffset);

			/* Returns readable bytes remaining. */
			size_t getBytesRemaining() const;

			const unsigned char* operator[](const size_t index) const { return &data[index]; }
			
		private:
			/* Reads bytes into destination. */
			size_t read(void* destination, const size_t bytes);

			const unsigned char* data;
		};
		
		/*
			Data exchanged between codex sockets upon connecting.
		*/
		class Handshake
		{
		public:
			Handshake();

			size_t write(WriteBuffer& buffer) const;
			size_t read(ReadBuffer& buffer);

			CodexType getCodexType() const;
			bool isValid() const;

		private:
			//Hidden attributes, set automatically
			VersionType handshakeVersion;//Version of handshake protocol.
			CodexType codexType;
			bool valid;
		};
	}
}