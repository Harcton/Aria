#pragma once
#include <vector>
#include "Codex/Device/ThreadedDevice.h"
#include "Codex/GPIO.h"

namespace codex
{
	namespace device
	{
		class Ublox_M8N : public codex::device::ThreadedDevice
		{
		public:
			Ublox_M8N();
			~Ublox_M8N() override;

			void setPins(const gpio::Pin transmit, const gpio::Pin receive);

			void onStart() override;
			void update() override;
			void onStop() override;

		private:

			mutable std::recursive_mutex mutex;

			gpio::Pin transmitPin;
			gpio::Pin receivePin;

			gpio::PinState transmitPinState;
			gpio::PinState receivePinState;

			enum class ReceiveState
			{
				//Connection establishment
				detectStreamBoundaries,

				//Main loop
				awaitingStartBit,		//Awaiting for the start bit (high state)
				receivingData,			//Receiving data bits between set intervals
				receivingParityBit,		//Receiving the parity bit
			};
			spehs::time::Time readInterval;
			int stopBitCount;
			int parityBitCount;
			ReceiveState receiveState;
			std::vector<uint8_t> receiveBuffer;
		};
	}
}