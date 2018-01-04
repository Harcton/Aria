#pragma once
#include <stdint.h>
#include <cstring>
#include <vector>
#include <string>
#include <type_traits>
#include <assert.h>
#include "Codex.h"
#include "GPIO.h"
#include "SpehsEngine/Core/Time.h"
#include "SpehsEngine/Core/Log.h"
#include "SpehsEngine/Core/HasMemberFunction.h"

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
		SPEHS_HAS_MEMBER_FUNCTION(write, has_write);
		SPEHS_HAS_MEMBER_FUNCTION(read, has_read);
		
		/* Byte endianness ordering */
		enum class Endianness : uint8_t
		{
			unknown = 0,
			little = 1,
			big = 2,
		};
		extern std::string getEndiannessAsString(const Endianness endianness);
		extern const Endianness hostByteOrder;
		extern const Endianness networkByteOrder;
		
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
			unknownProtocol,				//The protocol seems to be uknown?
		};

		/* Codex sockets add a packet type header into each packet. Packet type determines the contents of the packet. Currently a packet is solely limited to a single packet type. */
		enum class PacketType : uint8_t
		{
			invalid = 0,					//Not in use
			handshake,						//Packet contains codex defined handshake data
			disconnect,						//The remote end sends a packet to indicate disconnection
			undefined = 255,				//Sent data is user defined data, which is processed by a user defined receive handler.			
		};

		/*
			Currently codex uses the standard IPv4 endpoints that consist of a 32 bit address (presented in a string form) and a 16 bit port number.
		*/
		struct Endpoint
		{
			static const Endpoint invalid;
			Endpoint() : address(invalid.address), port(invalid.port) {}
			Endpoint(const AddressType _address, const PortType _port) : address(_address), port(_port) {}
			bool operator==(const Endpoint& other) const { return port == other.port && address == other.address; }
			AddressType address;
			PortType port;
		};
		Endpoint commandLineArgumentsToEndpoint(const int argc, const char** argv);

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
			virtual size_t getCapacity() const = 0;

			/* Returns the number of bytes written/read using the write/read method. */
			size_t getOffset() const;

		protected:
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
			
			void reserve(const size_t capacity);

			//Const class, has const write
			template<class T>
			typename std::enable_if<has_write<T, void(T::*)(WriteBuffer&) const>::value, void>::type write(const T& t)
			{
				t.write(*this);
			}
			//Const class, doesn't have const write
			template<class T>
			typename std::enable_if<!has_write<T, void(T::*)(WriteBuffer&) const>::value, void>::type write(const T& t)
			{
				SPEHS_ASSERT(false && "To use 'WriteBuffer::write<T>(const T&)' for a class type, the type T must have a 'void write(WriteBuffer&) const' method!");//NOTE: cannot use static assert because of g++ and SFINAE
			}
			//Mutable class, has mutable write
			template<class T>
			typename std::enable_if<has_write<T, void(T::*)(WriteBuffer&)>::value, void>::type write(T& t)
			{
				t.write(*this);
			}
			//Mutable class, has const write
			template<class T>
			typename std::enable_if<has_write<T, void(T::*)(WriteBuffer&) const>::value, void>::type write(T& t)
			{
				t.write(*this);
			}
			//Mutable class, doesn't have mutable or const write
			template<class T>
			typename std::enable_if<!has_write<T, void(T::*)(WriteBuffer&)>::value && !has_write<T, void(T::*)(WriteBuffer&) const>::value, void>::type write(T& t)
			{
				SPEHS_ASSERT(false && "To use 'WriteBuffer::write<T>(T&)' for a class type, the type T must have a 'void write(WriteBuffer&)' or 'void write(WriteBuffer&) const' method!");//NOTE: cannot use static assert because of g++ and SFINAE
			}
			//Isn't class
			template<typename T>
			typename std::enable_if<!std::is_class<T>::value, void>::type write (const T& t)
			{
				const size_t bytes = sizeof(T);
				data.resize(data.size() + bytes);

				if (hostByteOrder == networkByteOrder)
				{//Write in native order
					memcpy(&data[offset], &t, bytes);
					offset += bytes;
				}
				else
				{//Write in reversed order
					size_t endOffset = bytes;
					for (size_t i = 0; i < bytes; i++)
					{
						data[offset++] = ((const unsigned char*)&t)[--endOffset];
					}
				}
			}
			//Specialized cases
			void write(spehs::time::Time& t)
			{
				write(t.value);
			}
			void write(const spehs::time::Time& t)
			{
				write(t.value);
			}
			void write(std::string& t)
			{
				write(t.size());
				for (size_t i = 0; i < t.size(); i++)
					write(t[i]);
			}
			void write(const std::string& t)
			{
				write(t.size());
				for (size_t i = 0; i < t.size(); i++)
					write(t[i]);
			}

			const unsigned char* operator[](const size_t index) const { return &data[index]; }
			size_t getCapacity() const override { return data.capacity(); }

		private:

			/* Extends the buffer by increasing its size, without affecting the current offset. */
			bool extend(const size_t addedBytes);

			std::vector<unsigned char> data;
		};

		/*
			Does NOT own the underlying data!
		*/
		class ReadBuffer : public BufferBase
		{
		public:
			ReadBuffer(const void* pointedMemory, const size_t length);
			~ReadBuffer() override;

			//Is class, has mutable read
			template<class T>
			typename std::enable_if<has_read<T, void(T::*)(ReadBuffer&)>::value, void>::type read(T& t)
			{
				t.read(*this);
			}
			//Is class, doesn't have mutable read
			template<class T>
			typename std::enable_if<!has_read<T, void(T::*)(ReadBuffer&)>::value, void>::type read(T& t)
			{
				SPEHS_ASSERT(false && "To use 'ReadBuffer::read<T>(T&)' for a class type, the type T must have a 'void read(ReadBuffer&)' method!");//NOTE: cannot use static assert because of g++ and SFINAE
			}
			//Isn't class
			template<typename T>
			typename std::enable_if<!std::is_class<T>::value, void>::type read(T& t)
			{
				const size_t bytes = sizeof(T);
				if (offset + bytes > capacity)
				{
					spehs::log::warning("Cannot read past the buffer!");
					return;
				}

				if (hostByteOrder == networkByteOrder)
				{//Read in native byte order
					memcpy(&t, &data[offset], bytes);
				}
				else
				{//Read in reversed byte order
					size_t readOffset = offset + bytes;
					for (size_t i = 0; i < bytes; i++)
					{
						((unsigned char*)&t)[i] = data[--readOffset];
					}
				}

				offset += bytes;
			}
			//Specialized cases
			void read(spehs::time::Time& t)
			{
				read(t.value);
			}
			void read(std::string& t)
			{
				size_t sizeBytes;
				read(sizeBytes);
				t.resize(sizeBytes);
				for (size_t i = 0; i < sizeBytes; i++)
					read(t[i]);
			}

			/* Translates offset by a set amount of bytes. */
			void translate(const int translationOffset);

			/* Returns readable bytes remaining. */
			size_t getBytesRemaining() const;

			size_t getCapacity() const override { return capacity; }

			const unsigned char* operator[](const size_t index) const { return &data[index]; }
			
		private:

			size_t capacity;
			const unsigned char* data;
		};
		
		/*
			Data exchanged between codex sockets upon connecting.
		*/
		class Handshake
		{
		public:
			Handshake();

			void write(WriteBuffer& buffer) const;
			void read(ReadBuffer& buffer);

			bool isValid() const;

		private:
			//Hidden attributes, set automatically
			uint64_t magic;
			VersionType handshakeVersion;//Version of handshake protocol.
			bool valid;
		};
	}
}