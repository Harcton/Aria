#pragma once
#include <vector>
#include "Codex/Device/ThreadedDevice.h"
#include "Codex/GPIO.h"

namespace codex
{
	namespace device
	{
		class RS232_PinReader// : public codex::device::ThreadedDevice
		{
		public:
			RS232_PinReader();
			~RS232_PinReader();

			void setPin(const gpio::Pin pin);
			void setReadInterval(const time::TimeType interval);
			void setTransmissionUnitLength(const uint8_t length);
			/* Can have either 1 or 2 stop bits */
			void setStopBitCount(const int count);
			void setStreamBoundaryRequiredPatternRepetitionCount(int requiredRepetitions);
			void enableParityBitCheck();
			void disableParityBitCheck();

			void retrieveReceiveBuffer(std::vector<uint8_t>& buffer);

			void onStart();
			void update();
			void onStop();
			void start()
			{
				stopRequested = false;
				onStart();
				while (!stopRequested)
					update();
				onStop();
			}
			void stop() { stopRequested = true; }
			bool stopRequested;

		private:
			bool detectStreamBoundaries();

			mutable std::recursive_mutex mutex;

			gpio::Pin pin;

			enum class ReceiveState
			{
				invalid,
				awaitingStartBit,		//Awaiting for the start bit (high state)
				receivingData,			//Receiving data bits between set intervals
				receivingParityBit,		//Receiving the parity bit
				receivingStopBits,		//Receiving the stop bit(s)
			};
			time::TimeType readInterval;
			time::TimeType nextReadTime;
			gpio::PinState previousReadState;
			int stopBitCount;
			int streamBoundaryRequiredPatternRepetitionCount;
			bool parityCheckEnabled;
			ReceiveState receiveState;
			uint8_t transmittingUnit;
			uint8_t transmissionUnitLength;
			uint8_t transmittingUnitBitIndex;
			std::vector<uint8_t> receiveBuffer;
		};
	}
}