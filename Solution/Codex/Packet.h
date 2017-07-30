#pragma once
#include <stdint.h>
#include <boost/asio.hpp>

namespace codex
{
	class Packet
	{
	public:
		Packet() : data(nullptr), size(0)
		{

		}
		Packet(const size_t _size) : size(_size)
		{
			data = new unsigned char[size];
		}
		~Packet()
		{
			if (data)
				delete[] data;
		}

		bool resize(const size_t newSize)
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

		size_t getSize() const { return size; }
		const unsigned char* getData() const { return data; }

	private:
		unsigned char* data;
		size_t size;
	};
}