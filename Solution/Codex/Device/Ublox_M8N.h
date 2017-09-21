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
			bool headerReceived;
			time::TimeType readInterval;
			time::TimeType readTime;
			time::TimeType clockTimeStamp;
			uint8_t receiveBits;
			int receiveBitIndex;
			gpio::PinState transmissionSyncPreviousRxState;
			gpio::PinState clockReadSample;
			std::vector<uint8_t> receiveBuffer;
			std::vector<uint8_t> startSamples;
			int stopBitCount;
			int parityBitCount;
			ReceiveState receiveState;
		};
	}
}