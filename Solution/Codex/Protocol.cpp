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
		const uint16_t defaultAriaPort = 49842;

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

		void ReadBuffer::translate(const int translationOffset)
		{
			offset += translationOffset;
		}

		size_t ReadBuffer::getBytesRemaining() const
		{
			return capacity - offset;
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
			size_t offset = 0;
			offset += buffer.write(&endianness, sizeof(endianness));
			offset += buffer.write(&handshakeVersion, sizeof(handshakeVersion));
			return offset;
		}

		size_t Handshake::read(ReadBuffer& buffer)
		{//NOTE: buffer can contain invalid data! If so, set the valid boolean to false
			size_t offset = 0;
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

//		std::string ghostDirectory;
//		bool ghostRequestHandler(ReadBuffer& buffer)
//		{
//			std::string ghostName;
//			buffer.read(ghostName);
//
//			//Search for the specified ghost program on local drive. If ghost program exists, launch it and connect it with socket's remote endpoint
//			if (false)
//			{
//#ifdef _WIN32
//				// additional information
//				STARTUPINFO si;
//				PROCESS_INFORMATION pi;
//				// set the size of the structures
//				ZeroMemory(&si, sizeof(si));
//				si.cb = sizeof(si);
//				ZeroMemory(&pi, sizeof(pi));
//				// start the program up
//				CreateProcess(ghostDirectory + "/" + ghostName + ".exe", // format: "E:/Ohjelmointi/Projects/Aria/Solution/bin/Ghost0.exe"
//					"param0",		// Command line
//					NULL,           // Process handle not inheritable
//					NULL,           // Thread handle not inheritable
//					FALSE,          // Set handle inheritance to FALSE
//					CREATE_NEW_CONSOLE,//Creation flags
//					NULL,           // Use parent's environment block
//					NULL,           // Use parent's starting directory 
//					&si,            // Pointer to STARTUPINFO structure
//					&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
//				);
//				// Close process and thread handles.
//				CloseHandle(pi.hProcess);
//				CloseHandle(pi.hThread);
//#endif
//			}
//			return false;//=do not start receiving again
//		}
//
//		std::mutex requestGhostMutex;		
//		std::atomic<bool> requestGhostResponseReceived(false);
//		const uint64_t magicYes = 0xBAABAABBBBADAB00;
//		uint64_t requestGhostReceivedResponse;
//		std::string requestGhostReceivedResponseAddress;
//		uint16_t requestGhostReceivedResponsePort;
//		bool requestGhostInternalReceiveHandler(ReadBuffer& buffer)
//		{
//			requestGhostResponseReceived = true;
//			buffer.read(requestGhostReceivedResponse);
//			if (requestGhostReceivedResponse == magicYes)
//			{//Read the rest
//				buffer.read(requestGhostReceivedResponseAddress);
//				buffer.read(requestGhostReceivedResponsePort);
//			}
//			return false;
//		}
//		bool requestGhost(SocketTCP& socket, const std::string& ghostName)
//		{
//			//TODO: do not limit this function for just one thread...
//			if (!requestGhostMutex.try_lock())
//			{
//				log::warning("requestGhost() failed: another thread is already runnig this process!");
//				return false;
//			}
//			requestGhostMutex.unlock();
//			std::lock_guard<std::mutex> lock(requestGhostMutex);
//
//			if (!socket.isConnected())
//			{
//				log::warning("requestGhost() failed: passed socket is not connected to an endpoint! Connect first, then request the ghost!");
//				return false;
//			}
//			if (socket.isReceiving())
//			{
//				log::warning("requestGhost() failed: passed socket is already receiving!");
//				return false;
//			}
//
//			WriteBuffer buffer(Endianness::equal);
//			buffer.write(ghostName);
//			if (!socket.sendPacket())
//			{
//				log::info("requestGhost() failed: sending a packet failed!");
//				return false;
//			}
//
//			requestGhostReceivedResponse = false;
//			socket.startReceiving(std::bind(requestGhostInternalReceiveHandler, std::placeholders::_1));
//			codex::time::TimeType startTime = time::getRunTime();
//			while (!requestGhostReceivedResponse)
//			{
//				codex::time::delay(codex::time::milliseconds(1));
//				if (time::getRunTime() - startTime > codex::time::seconds(5))
//				{
//					codex::log::info("requestGhost() failed: the remote socket did not respond within the given time window.");
//					socket.stopReceiving();
//					return false;
//				}
//			}
//
//			if (requestGhostReceivedResponse == magicYes)
//			{//Ghost has been deployed to the specified endpoint
//				const std::string remoteAddress = socket.getRemoteAddress();
//				const uint16_t remotePort = socket.getRemotePort();
//				socket.disconnect();
//
//				//Connect to ghost
//				if (!socket.connect(requestGhostReceivedResponseAddress, requestGhostReceivedResponsePort))
//				{
//					codex::log::info("requestGhost() failed: could not connect to the provided ghost.");
//					//Try to reconnect to the previously connected endpoint
//					if (!socket.connect(remoteAddress, remotePort))
//						codex::log::info("requestGhost() failed to reconnect back to the previous endpoint!");
//
//					return false;
//				}
//
//			}
//			else
//			{
//				codex::log::info("requestGhost() failed: the remote socket responded, but it could not currently provide a ghost with the specified name.");
//				return true;
//			}
//		}
	}
}