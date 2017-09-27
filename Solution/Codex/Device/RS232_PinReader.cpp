#include "RS232_PinReader.h"
#include "Codex/Log.h"
#include <iostream>
#include <cassert>

namespace codex
{
	namespace device
	{
		int debugLogLevel = 1;

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
		}

		RS232_PinReader::~RS232_PinReader()
		{
		}

		void RS232_PinReader::setPin(const gpio::Pin _pin)
		{
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
			receiveBuffer.clear();
		}

		void RS232_PinReader::onStart()
		{
			detectStreamBoundaries();
		}

		void RS232_PinReader::update()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			const gpio::PinState readState = gpio::read(pin);

			if (receiveState == ReceiveState::awaitingStartBit)
			{
				if (readState == gpio::PinState::high)
				{//Start bit received, proceed to receive data

					//Synchronize read times from the point of receiving the start bit
					nextReadTime = time::getRunTime() + readInterval + readInterval / 2;

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
					nextReadTime = time::getRunTime() + readInterval + readInterval / 2;
				}
				else if (time::getRunTime() >= nextReadTime)
				{
					bitReceived = true;
					nextReadTime += readInterval;
				}

				if (bitReceived)
				{//Reached the next read time mark without the pin state changing
					
					if (debugLogLevel >= 3)
						log::info("RS232_PinReader data bit received.");

					if (readState == gpio::PinState::high)
						transmittingUnit |= 0x01 << transmittingUnitBitIndex;

					if (++transmittingUnitBitIndex == transmissionUnitLength)
					{//Read the entire transmitting unit

						if (debugLogLevel >= 3)
							log::info("RS232_PinReader received unit: " + std::to_string(transmittingUnit));

						receiveState = ReceiveState::receivingParityBit;
					}
				}
			}
			else if (receiveState == ReceiveState::receivingParityBit)
			{
				bool bitReceived = false;
				if (readState != previousReadState)
				{
					bitReceived = true;
					nextReadTime = time::getRunTime() + readInterval + readInterval / 2;
				}
				else if (time::getRunTime() >= nextReadTime)
				{
					bitReceived = true;
					nextReadTime += readInterval;
				}

				if (bitReceived)
				{//Read a parity bit from the stream
					
					if (debugLogLevel >= 2)
						log::info("RS232_PinReader parity bit received.");

					if (parityCheckEnabled)
					{
						int checksum = 0;
						for (uint8_t i = 0; i < transmissionUnitLength; i++)
						{
							if (((0x01 << i) & transmittingUnit) != 0)
								checksum++;
						}

						if ((checksum % 2) == 1/*odd*/ && readState == gpio::PinState::low/*odd*/ || (checksum % 2) == 0/*even*/ && readState == gpio::PinState::high/*even*/)
						{//Checksum checks out == successfully received
							receiveBuffer.push_back(transmittingUnit);
							if (debugLogLevel >= 2)
								log::info("RS232_PinReader checksum checks out.");
						}
						else
						{//Error
							if (debugLogLevel >= 1)
								log::info("RS232_PinReader checksum error!");
						}
					}
					else
					{//Do not check checksum
						receiveBuffer.push_back(transmittingUnit);
					}

					receiveState = ReceiveState::receivingStopBits;
				}
			}
			else if (receiveState == ReceiveState::receivingStopBits)
			{
				if (readState != previousReadState || time::getRunTime() >= nextReadTime)
				{
					if (readState == gpio::PinState::low)
					{
						if (debugLogLevel >= 2)
							log::info("RS232_PinReader stop bit received.");
						receiveState = ReceiveState::awaitingStartBit;
					}
					else
					{//No stop bit?
						if (debugLogLevel >= 1)
							log::info("RS232_PinReader no stop bit received. Receive buffer size: " + std::to_string(receiveBuffer.size()));
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

			if (debugLogLevel >= 1)
				log::info("RS232_PinReader: Starting to detect stream boundaries. Set options: baud rate '" + std::to_string(int(1.0f / ((float)readInterval / (float)time::conversionRate::second))) + "', parity check " + (parityCheckEnabled ? "enabled" : "disabled"));
			
			std::vector<gpio::PinState> history;

			//Synchronize reading
			while (gpio::read(pin) == gpio::PinState::high)
			{
				//Blocks until in low state
			}
			while (gpio::read(pin) == gpio::PinState::low)
			{
				//Blocks until in high state
			}
			nextReadTime = time::getRunTime() + readInterval + readInterval / 2;
			if (debugLogLevel >= 1)
				log::info("RS232_PinReader: Initial transmitter pin state detected. Reading is now synchronized.");

			int analyzeCount = 0;
			int changingEdgeSamples = 0;
			int timedSamples = 0;
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
						nextReadTime = time::getRunTime() + readInterval + readInterval / 2;
						changingEdgeSamples++;
						break;
					}
					else if (time::getRunTime() >= nextReadTime)
					{//Reached the next read time mark without the pin state changing
						history.push_back(readState);
						nextReadTime += readInterval;
						timedSamples++;
						break;
					}
				}

				stopBitCount = 1/*at least 1 stop bit*/ + analyzeCount % 2/* + up to one extra stop bit*/;
				const int trailerBitCount = 1/*parity*/ + stopBitCount;
				const int sequenceLength = 1/*start bit*/ + transmissionUnitLength/*data bits*/ + trailerBitCount;
				static const int minRequiredSequenceCount = streamBoundaryRequiredPatternRepetitionCount;//At least this many sequences are required to make credible analyzation
				if (history.size() > sequenceLength * minRequiredSequenceCount)
				{////Analyze...
					/*
						Look for a pattern:
						1 start bit (high)
						8 data bits (x)
						0-1 parity bits (x)
						1-2 stop bits (low)
					*/
					std::vector<int/*history index*/> potentialStartBits;
					const int endOffset = sequenceLength - 1;
					const int requiredPatternLength = history.size() / sequenceLength - 1;
					if (stopBitCount == 1)
					{
						for (int i = 0; i + endOffset < history.size(); i++)
						{
							if (history[i] == gpio::PinState::high && history[i + endOffset] == gpio::PinState::low)
								potentialStartBits.push_back(i);
						}
					}
					else if (stopBitCount == 2)
					{
						for (int i = 0; i + endOffset < history.size(); i++)
						{
							if (history[i] == gpio::PinState::high && history[i + endOffset - 1] == gpio::PinState::low && history[i + endOffset] == gpio::PinState::low)
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

					if (debugLogLevel >= 1)
					{
						log::info(
							"RS232_PinReader: Analyze #" + std::to_string(++analyzeCount) + " Results: "
							+ std::to_string(history.size()) + " history samples, "
							+ std::to_string(potentialStartBits.size()) + " potential start bits, "
							+ std::to_string(validPatterns.size()) + " valid patterns. "
							+ " Longest sequence pattern: " + std::to_string(longestSequencePattern)
							+ ", Transmitting unit length: " + std::to_string(transmissionUnitLength)
							+ ", Parity check " + (parityCheckEnabled ? "enabled" : "disabled")
							+ ", Stop bits: " + std::to_string(stopBitCount)
							+ ", Changing edge samples: " + std::to_string(changingEdgeSamples)
							+ ", Timed samples: " + std::to_string(timedSamples));
					}

					if (time::getRunTime() >= nextReadTime)
					{//Analyze took too long! Missed the next read!
						if (debugLogLevel >= 1)
							log::info("RS232_PinReader: Analyze took too long! (-" + std::to_string(time::toMilliseconds(time::getRunTime() - nextReadTime)) + " ms) Sample history has been reset.");
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
						const time::TimeType t1 = time::getRunTime();
						log::info("RS232_PinReader skipping the next " + std::to_string(readSkipCount) + " reads. Estimated skip time: " + std::to_string((nextReadTime - time::getRunTime() + time::TimeType(readSkipCount - 1) * readInterval) / time::conversionRate::nanosecond) + " ns.");
						log::info("History size: " + std::to_string(history.size()));
						log::info("Pattern begins at: " + std::to_string(validPatterns[0]));
						log::info("Sequence length: " + std::to_string(sequenceLength));

						const gpio::PinState initialPinState = gpio::read(pin);
						while (readSkipCount > 0)
						{
							const gpio::PinState readState = gpio::read(pin);
							if (readState != initialPinState)
							{//State changed, synchronize clock at this point
								nextReadTime = time::getRunTime() + readInterval + readInterval / 2;
								readSkipCount--;
							}
							else if (time::getRunTime() >= nextReadTime)
							{//Reached the next read time mark without the pin state changing
								nextReadTime += readInterval;
								readSkipCount--;
							}
						}
						//nextReadTime += (readSkipCount - 1) * readInterval;
						//while (time::getRunTime() < nextReadTime)
						//{
						//	//Blocks until all skips have been made
						//}
						nextReadTime += readInterval;
						if (gpio::read(pin) != gpio::low)
						{
							log::info("RS232_PinReader failed to arrive at the stop bit!");
							return false;
						}
						
						receiveState = ReceiveState::awaitingStartBit;
						if (debugLogLevel >= 1)
							log::info("RS232_PinReader: successfully detected stream boundaries!");
						return true;
					}
				}
			}
			return false;
		}
	}
}