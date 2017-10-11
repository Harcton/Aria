#include "Codex/Device/RS232_PinReader.h"
#include "Codex/StringOperations.h"
#include "Codex/Log.h"
#include <iostream>
#include <cassert>

namespace codex
{
	namespace device
	{
		/*
			Debug log levels
				1. Only log when NOT running
				2. Log when running, only necessities
				3. Everything not within the above description
		*/
		const int debugLogLevel = 1;
		const gpio::PinState startBitState = gpio::PinState::low;
		const gpio::PinState stopBitState = gpio::PinState::high;
		const gpio::PinState oddParityBitState = gpio::PinState::low;
		const gpio::PinState evenParityBitState = gpio::PinState::high;
		const gpio::PinState enabledDataBitState = gpio::PinState::high;

		RS232_PinReader::RS232_PinReader()
			: pin(gpio::pin_none)
			, readInterval(time::seconds(1.0f / 9600.0f))
			, nextReadTime(0)
			, previousReadState(gpio::PinState::invalid)
			, stopBitCount(1)
			, streamBoundaryRequiredPatternRepetitionCount(5)
			, parityCheckEnabled(false)
			, receiveState(ReceiveState::invalid)
			, transmissionUnitLength(8)
		{
			receiveBuffer.reserve(65536);
		}

		RS232_PinReader::~RS232_PinReader()
		{
		}

		void RS232_PinReader::setPin(const gpio::Pin _pin)
		{
			gpio::setPinAsInput(_pin);
			std::lock_guard<std::recursive_mutex> lock(mutex);
			pin = _pin;
		}

		void RS232_PinReader::setReadInterval(const time::TimeType interval)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			readInterval = interval;
		}

		void RS232_PinReader::setTransmissionUnitLength(const uint8_t length)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			transmissionUnitLength = length;
		}

		void RS232_PinReader::setStopBitCount(const int count)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			if (count == 2)
				stopBitCount = 2;
			else
				stopBitCount = 1;
		}

		void RS232_PinReader::setStreamBoundaryRequiredPatternRepetitionCount(int requiredRepetitions)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			if (requiredRepetitions < 2)
				streamBoundaryRequiredPatternRepetitionCount = 2;
			else
				streamBoundaryRequiredPatternRepetitionCount = requiredRepetitions;
		}

		void RS232_PinReader::enableParityBitCheck()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			parityCheckEnabled = true;
		}

		void RS232_PinReader::disableParityBitCheck()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			parityCheckEnabled = false;
		}

		void RS232_PinReader::retrieveReceiveBuffer(std::vector<uint8_t>& buffer)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			buffer.insert(buffer.end(), receiveBuffer.begin(), receiveBuffer.end());
			receiveBuffer.resize(0);
		}

		void RS232_PinReader::onStart()
		{
			while (!detectStreamBoundaries())
			{
				//Blocks
			}
		}

		void RS232_PinReader::update()
		{
			//std::lock_guard<std::recursive_mutex> lock(mutex);
			const gpio::PinState readState = gpio::read(pin);

			if (receiveState == ReceiveState::awaitingStartBit)
			{
				if (readState == startBitState)
				{//Start bit received, proceed to receive data

					//Synchronize read times from the point of receiving the start bit
					nextReadTime = time::now() + readInterval + readInterval / 2;

					receiveState = ReceiveState::receivingData;
					transmittingUnit = 0x00;
					transmittingUnitBitIndex = 0;
					if (debugLogLevel >= 2)
						log::info("RS232_PinReader start bit received.");
				}
			}
			else if (receiveState == ReceiveState::receivingData)
			{
				assert(transmittingUnitBitIndex < transmissionUnitLength);
				
				bool bitReceived = false;
				if (readState != previousReadState)
				{
					bitReceived = true;
					nextReadTime = time::now() + readInterval + readInterval / 2;
				}
				else if (time::now() >= nextReadTime)
				{
					bitReceived = true;
					nextReadTime += readInterval;
				}

				if (bitReceived)
				{//Reached the next read time mark without the pin state changing
					
					if (debugLogLevel >= 3)
						log::info("RS232_PinReader data bit received.");

					if (readState == enabledDataBitState)
						transmittingUnit |= 0x01 << transmittingUnitBitIndex;

					if (++transmittingUnitBitIndex == transmissionUnitLength)
					{//Read the entire transmitting unit

						if (debugLogLevel >= 3)
							log::info("RS232_PinReader received unit: " + std::to_string(transmittingUnit));

						if (parityCheckEnabled)
							receiveState = ReceiveState::receivingParityBit;
						else
						{
							receiveBuffer.push_back(transmittingUnit);
							receiveState = ReceiveState::receivingStopBits;
						}
					}
				}
			}
			else if (receiveState == ReceiveState::receivingParityBit)
			{
				assert(parityCheckEnabled);
				bool bitReceived = false;
				if (readState != previousReadState)
				{
					bitReceived = true;
					nextReadTime = time::now() + readInterval + readInterval / 2;
				}
				else if (time::now() >= nextReadTime)
				{
					bitReceived = true;
					nextReadTime += readInterval;
				}

				if (bitReceived)
				{//Read a parity bit from the stream
					
					if (debugLogLevel >= 2)
						log::info("RS232_PinReader parity bit received.");

					int checksum = 0;
					for (uint8_t i = 0; i < transmissionUnitLength; i++)
					{
						if (((0x01 << i) & transmittingUnit) != 0)
							checksum++;
					}

					if ((checksum % 2) == 1/*odd*/ && readState == oddParityBitState || (checksum % 2) == 0/*even*/ && readState == evenParityBitState)
					{//Checksum checks out == successfully received
						receiveBuffer.push_back(transmittingUnit);
						if (debugLogLevel >= 2)
							log::info("RS232_PinReader checksum checks out.");
					}
					else
					{//Error
						if (debugLogLevel >= 2)
							log::info("RS232_PinReader checksum error!");
					}

					receiveState = ReceiveState::receivingStopBits;
				}
			}
			else if (receiveState == ReceiveState::receivingStopBits)
			{
				if (readState != previousReadState || time::now() >= nextReadTime)
				{
					if (readState == stopBitState)
					{
						if (debugLogLevel >= 2)
							log::info("RS232_PinReader stop bit received.");
						receiveState = ReceiveState::awaitingStartBit;
					}
					else
					{//No stop bit?
						
					 	std::string hexStr;
					 	std::string charStr;
					 	hexStr.reserve(3 * receiveBuffer.size());
					 	charStr.reserve(receiveBuffer.size());
						const int printSize = receiveBuffer.size() > 1024 ? 1024 : receiveBuffer.size();
					 	for (size_t i = 0; i < printSize; i++)
					 	{
					 		hexStr += " " + toHexString(receiveBuffer[i]);
					 		charStr += receiveBuffer[i];
							if (charStr.size() >= 32)
							{
								codex::log::info(hexStr + charStr);
								hexStr.clear();
								charStr.clear();
							}
					 	}
					 	codex::log::info(hexStr + charStr);

						if (debugLogLevel >= 1)
							log::info("RS232_PinReader no stop bit received. Receive buffer size: " + std::to_string(receiveBuffer.size()));
						receiveBuffer.resize(0);
						receiveState = ReceiveState::invalid;
					}
				}
			}
			else
			{
				stop();
			}

			previousReadState = readState;
		}

		void RS232_PinReader::onStop()
		{

		}
		
		bool RS232_PinReader::detectStreamBoundaries()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			const time::TimeType detectStreamBoundariesBeginTime = time::now();

			if (debugLogLevel >= 1)
				log::info("RS232_PinReader: Starting to detect stream boundaries. Set options: baud rate '" + std::to_string(int(1.0f / ((float)readInterval / (float)time::conversionRate::second))) + "', parity check " + (parityCheckEnabled ? "enabled" : "disabled"));
			
			std::vector<gpio::PinState> history(16384);
			int analyzeCount = 0;
			int changingEdgeSamples = 0;
			int timedSamples = 0;
			const int trailerBitCount = (parityCheckEnabled ? 1 : 0) + stopBitCount;
			const int sequenceLength = 1/*start bit*/ + transmissionUnitLength/*data bits*/ + trailerBitCount;
			static const int minRequiredSequenceCount = streamBoundaryRequiredPatternRepetitionCount;//At least this many sequences are required to make credible analyzation

			//Synchronize reading
			while (gpio::read(pin) == gpio::PinState::high)
			{
				//Blocks until in low state
			}
			while (gpio::read(pin) == gpio::PinState::low)
			{
				//Blocks until in high state
			}
			nextReadTime = time::now() + readInterval + readInterval / 2;
			if (debugLogLevel >= 2)
				log::info("RS232_PinReader: Initial transmitter pin state detected. Reading is now synchronized.");

			while (true)
			{
				//Wait until reached next read time or state changes before
				const gpio::PinState initialPinState = gpio::read(pin);
				while (true)
				{
					const gpio::PinState readState = gpio::read(pin);
					if (readState != initialPinState)
					{//State changed, synchronize clock at this point
						history.push_back(readState);
						nextReadTime = time::now() + readInterval + readInterval / 2;
						changingEdgeSamples++;
						break;
					}
					else if (time::now() >= nextReadTime)
					{//Reached the next read time mark without the pin state changing
						history.push_back(readState);
						nextReadTime += readInterval;
						timedSamples++;
						break;
					}
				}

				if (history.size() > sequenceLength * minRequiredSequenceCount)
				{////Analyze...
					assert(history.size() >= minRequiredSequenceCount * 2);
					/*
						Look for a pattern:
						1 start bit (low)
						8 data bits (x)
						0-1 parity bits (x)
						1-2 stop bits (high)
					*/
					std::vector<int/*history index*/> potentialStartBits;
					const int endOffset = sequenceLength - 1;
					const int requiredPatternLength = history.size() / sequenceLength - 1;
					const int lastTestedStartBitIndex = history.size() - endOffset;
					if (stopBitCount == 1)
					{
						for (int i = 0; i < lastTestedStartBitIndex; i++)
						{
							if (history[i] == startBitState && history[i + endOffset] == stopBitState)
								potentialStartBits.push_back(i);
						}
					}
					else if (stopBitCount == 2)
					{
						for (int i = 0; i < lastTestedStartBitIndex; i++)
						{
							if (history[i] == startBitState && history[i + endOffset - 1] == stopBitState && history[i + endOffset] == stopBitState)
								potentialStartBits.push_back(i);
						}
					}
					else
					{
						assert(false);
					}

					/*
					Now, we have all recorded history indices that could potentially be start bits.
					These candidates are in ascending order.
					Eliminate the ones that do not appear in a consistent pattern.
					*/
					std::vector<int/*start bit history index*/> validPatterns;
					int longestSequencePattern = 0;
					while (potentialStartBits.size() >= requiredPatternLength)
					{
						//Check if the potential front start bit is a valid sequence pattern
						std::vector<int> patternSequenceIndices;
						patternSequenceIndices.push_back(potentialStartBits.front());
						int expectedNextIndex = potentialStartBits.front() + sequenceLength;

						for (size_t r = 1; r < potentialStartBits.size(); r++)
						{
							if (potentialStartBits[r] == expectedNextIndex)
							{//Set the next expected index
								expectedNextIndex = potentialStartBits[r] + sequenceLength;
								patternSequenceIndices.push_back(potentialStartBits[r]);
							}
							else if (potentialStartBits[r] > expectedNextIndex)
							{//Expected index was not found
								break;
							}
						}

						if (patternSequenceIndices.size() >= requiredPatternLength)
						{//Found a valid pattern
							validPatterns.push_back(patternSequenceIndices.front());
						}

						if (patternSequenceIndices.size() > longestSequencePattern)
							longestSequencePattern = patternSequenceIndices.size();

						//Remove sequence start indices
						for (size_t p = 0; p < patternSequenceIndices.size(); p++)
						{
							bool found = false;
							for (size_t s = 0; s < potentialStartBits.size(); s++)
							{
								if (patternSequenceIndices[p] == potentialStartBits[s])
								{
									potentialStartBits.erase(potentialStartBits.begin() + s);
									found = true;
									break;
								}
							}
							assert(found);
						}
					}

					if (debugLogLevel >= 2)
					{
						log::info(
							"RS232_PinReader: Analyze #" + std::to_string(++analyzeCount) + " Results: "
							+ std::to_string(history.size()) + " history samples, "
							+ std::to_string(potentialStartBits.size()) + " potential start bits, "
							+ std::to_string(validPatterns.size()) + " valid patterns. "
							+ " Longest sequence pattern: " + std::to_string(longestSequencePattern)
							+ ", Transmitting unit length: " + std::to_string(transmissionUnitLength)
							+ ", Parity bit/check " + (parityCheckEnabled ? "enabled" : "disabled")
							+ ", Stop bits: " + std::to_string(stopBitCount)
							+ ", Changing edge samples: " + std::to_string(changingEdgeSamples)
							+ ", Timed samples: " + std::to_string(timedSamples));
					}

					if (time::now() >= nextReadTime)
					{//Analyze took too long! Missed the next read!
						if (debugLogLevel >= 1)
							log::info("RS232_PinReader: Analyze took too long! (-" + std::to_string(time::toMilliseconds(time::now() - nextReadTime)) + " ms) Sample history has been reset.");
						history.clear();
						validPatterns.clear();
						nextReadTime = std::numeric_limits<time::TimeType>::max();//Wait for pin state to change before reading again
						changingEdgeSamples = 0;
						timedSamples = 0;
					}
					else if (validPatterns.size() == 1)
					{//Found exactly one valid pattern!

						//Wait until the end of the currently transmitting sequence (at the time of stop bit)
						int readSkipCount = sequenceLength - ((history.size() - validPatterns[0]) % sequenceLength);
						const time::TimeType t1 = time::now();
						//log::info("RS232_PinReader skipping the next " + std::to_string(readSkipCount) + " reads. Estimated skip time: " + std::to_string((nextReadTime - time::now() + time::TimeType(readSkipCount - 1) * readInterval) / time::conversionRate::nanosecond) + " ns.");
						//log::info("History size: " + std::to_string(history.size()));
						//log::info("Pattern begins at: " + std::to_string(validPatterns[0]));
						//log::info("Sequence length: " + std::to_string(sequenceLength));

						const gpio::PinState initialPinState = gpio::read(pin);
						while (readSkipCount > 0)
						{
							const gpio::PinState readState = gpio::read(pin);
							if (readState != initialPinState)
							{//State changed, synchronize clock at this point
								nextReadTime = time::now() + readInterval + readInterval / 2;
								readSkipCount--;
							}
							else if (time::now() >= nextReadTime)
							{//Reached the next read time mark without the pin state changing
								nextReadTime += readInterval;
								readSkipCount--;
							}
						}
						//nextReadTime += (readSkipCount - 1) * readInterval;
						//while (time::now() < nextReadTime)
						//{
						//	//Blocks until all skips have been made
						//}
						nextReadTime += readInterval;
						if (gpio::read(pin) != stopBitState)
						{
							log::info("RS232_PinReader failed to arrive at the stop bit!");
							return false;
						}
						
						receiveState = ReceiveState::awaitingStartBit;
						if (debugLogLevel >= 2)
						{
							const time::TimeType detectStreamBoundariesDuration = time::now() - detectStreamBoundariesBeginTime;
							std::string str = "Took ";
							if (detectStreamBoundariesDuration > time::conversionRate::second)
								str += std::to_string(time::toSeconds(detectStreamBoundariesDuration)) + " seconds";
							else if (detectStreamBoundariesDuration > time::conversionRate::millisecond)
								str += std::to_string(time::toMilliseconds(detectStreamBoundariesDuration)) + " milliseconds";
							else if (detectStreamBoundariesDuration > time::conversionRate::microsecond)
								str += std::to_string(time::toMicroseconds(detectStreamBoundariesDuration)) + " microseconds";
							else if (detectStreamBoundariesDuration > time::conversionRate::nanosecond)
								str += std::to_string(time::toNanoseconds(detectStreamBoundariesDuration)) + " nanoseconds";
							log::info("RS232_PinReader: successfully detected stream boundaries! " + str);
						}
						return true;
					}
				}
			}
			return false;
		}
	}
}