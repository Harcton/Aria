#pragma once
#include <stdint.h>
#include <cstring>
#include <assert.h>
#include "Log.h"

namespace codex
{
	namespace protocol
	{
		typedef uint16_t OfferVersionType;

		typedef uint32_t VersionType;
		class WriteBuffer;
		class ReadBuffer;

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

		


		namespace aria
		{//Aria protocol
			
			class Serializable
			{
			public:
				/*
					A single byte type is not affected by endianness, yet large enough to cover our needs.
					To add new serializables:
						1. Add the type enumeration. The enumeration name should match the class name.
						2. Add instantiation to deserializeFromReadBuffer implementation.
						3. Add CODEX_ARIA_SERIALIZABLE(ClassName) macro to the serializable class declaration (under public).
						4. Add write() and read() pure virtual method implementations. The implementations must call the base class implementations first!
				*/
				enum class Type : unsigned char
				{
					Handshake,
					String,
					GhostQuery,
					GhostOffer,
				};
				/* Reads the Serializable Type from buffer, creates serializable instance and reads it. */
				static Serializable* deserializeFromReadBuffer(ReadBuffer& buffer);				
			public:
				virtual Type getSerializableType() const = 0;
				virtual Serializable* clone() const = 0;
				virtual size_t write(WriteBuffer& buffer) const
				{
					const Type type = getSerializableType();
					buffer.write(&type, sizeof(type));
					return sizeof(type);
				}
				virtual size_t read(ReadBuffer& buffer)
				{
					Type type;
					buffer.read(&type, sizeof(type));
					assert(type == getSerializableType());
					return sizeof(type);
				}
			public:
			};
/////////////////////////////////////////////////////////////////////
#define CODEX_PROTOCOL_ARIA_SERIALIZABLE(ClassName) \
			Type getSerializableType() const override { return Serializable::Type::ClassName; } \
			Serializable* clone() const override { return new ClassName(); } \
/////////////////////////////////////////////////////////////////////

			class Handshake : public Serializable
			{
			public:
				CODEX_PROTOCOL_ARIA_SERIALIZABLE(Handshake)
			public:
				Handshake();

				size_t write(WriteBuffer& buffer) const override;
				size_t read(ReadBuffer& buffer) override;

				Endianness getEndianness() const;
				bool isValid() const;

			private:
				//Hidden attributes, set automatically
				uint16_t endianness;//2 Endianness bytes to determine the system's endianness
				VersionType handshakeVersion;//Version of handshake protocol.
				bool valid;
			};
						
			class String : public std::string, public Serializable
			{
			public:
				CODEX_PROTOCOL_ARIA_SERIALIZABLE(String)
			public:
				String() = default;
				String(const char* str) : std::string(str) {}
				size_t write(WriteBuffer& buffer) const override
				{
					size_t offset = Serializable::write(buffer);
					uint32_t length = size();
					offset += buffer.write(&length, sizeof(length));
					return offset;
				}
				size_t read(ReadBuffer& buffer) override
				{
					size_t offset = Serializable::read(buffer);
					return offset;
				}
			};

			class GhostQuery : public Serializable
			{
			public:
				CODEX_PROTOCOL_ARIA_SERIALIZABLE(GhostQuery)
			public:
				GhostQuery(){}
				size_t write(WriteBuffer& buffer) const override
				{
					size_t offset = Serializable::write(buffer);
					offset += ghostName.write(buffer);
					return offset;
				}
				size_t read(ReadBuffer& buffer) override
				{
					size_t offset = Serializable::read(buffer);
					offset += ghostName.read(buffer);
					return offset;
				}
				String ghostName;
			};

			class GhostOffer : public Serializable
			{
			public:
				CODEX_PROTOCOL_ARIA_SERIALIZABLE(GhostOffer)
			public:
				GhostOffer(){}
				size_t write(WriteBuffer& buffer) const override
				{
					size_t offset = Serializable::write(buffer);
					offset += buffer.write(&port, sizeof(port));
					return offset;
				}
				size_t read(ReadBuffer& buffer) override
				{
					size_t offset = Serializable::read(buffer);
					offset += buffer.read(&port, sizeof(port));
					return offset;
				}
				uint16_t port;
			};
		}
	}
}