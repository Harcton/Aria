#pragma once
#include <stdint.h>
#include <cstring>
#include <assert.h>
#include "Log.h"

namespace codex
{
	class SocketTCP;
	namespace protocol
	{
		typedef uint16_t OfferVersionType;

		typedef uint32_t VersionType;
		class WriteBuffer;
		class ReadBuffer;

		extern const uint16_t defaultAriaPort;

		enum class ContactPacketHeaderType : uint16_t
		{/////////////////////////0xFF00 endianness detection
			requestOffer	= 0xFF01,
			offer			= 0xFF02,
			acceptOffer		= 0xFF03,
		};

		enum class Endianness : unsigned char
		{//Byte endianness ordering
			equal = 1,
			inverted = 2,
		};



		class BufferBase
		{
		public:
			BufferBase();
			virtual ~BufferBase() = 0;

			//Byte ordering setting
			bool hasReversedByteOrder() const { return reversedByteOrder; }

			/* Returns the total capacity of the write buffer. */
			size_t getCapacity() const;

			/* Returns the number of bytes written using the write method. */
			size_t getWrittenSize() const;

		protected:
			size_t capacity;
			size_t offset;
			bool reversedByteOrder;
		};

		/*
			Write buffers can only be extended, but never contracted.
			Makes it possible to write in inversed byte order.
			Owns the underlying data.
		*/
		class WriteBuffer : public BufferBase
		{
		public:
			WriteBuffer(const Endianness writeEndianness);
			WriteBuffer(const Endianness writeEndianness, const size_t _size);
			~WriteBuffer() override;
			
			/* Extends the buffer by increasing its size, without affecting the current offset. */
			bool extend(const size_t addedBytes);

			/* Writes to data*/
			size_t write(const void* buffer, const size_t length);
			size_t write(const uint8_t value);
			size_t write(const int8_t value);
			size_t write(const uint16_t value);
			size_t write(const int16_t value);
			size_t write(const uint32_t value);
			size_t write(const int32_t value);
			size_t write(const uint64_t value);
			size_t write(const int64_t value);
			size_t write(const bool value);
			size_t write(const std::string& value);

			const unsigned char* operator[](const size_t index) const { return &data[index]; }

		private:
			unsigned char* data;
		};

		/*
			Does NOT own the underlying data!
			Makes it possible to read the underlying data in correct byte order.
		*/
		class ReadBuffer : public BufferBase
		{
		public:
			ReadBuffer(const void* pointedMemory, const size_t length);
			~ReadBuffer() override;

			/* Reads bytes into destination, taking into account byte endianness. */
			size_t read(void* destination, const size_t bytes);
			size_t read(uint8_t& value);
			size_t read(int8_t& value);
			size_t read(uint16_t& value);
			size_t read(int16_t& value);
			size_t read(uint32_t& value);
			size_t read(int32_t& value);
			size_t read(uint64_t& value);
			size_t read(int64_t& value);
			size_t read(bool& value);
			size_t read(std::string& value);

			/* Translates offset by set amount of bytes. */
			void translate(const int translationOffset);

			/* Returns readable bytes remaining. */
			size_t getBytesRemaining() const;
						
			/* Byte ordering setting */
			void setReversedByteOrder(const bool isReversed) { reversedByteOrder = isReversed; }
			void toggleReversedByteOrder() { reversedByteOrder = !reversedByteOrder; }
			void enableReversedByteOrder() { reversedByteOrder = true; }
			void disableReversedByteOrder() { reversedByteOrder = false; }

		private:
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

			Endianness getEndianness() const;
			bool isValid() const;

		private:
			//Hidden attributes, set automatically
			uint16_t endianness;//2 Endianness bytes to determine the system's endianness
			VersionType handshakeVersion;//Version of handshake protocol.
			bool valid;
		};


	}
}