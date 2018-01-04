#include "Ublox_M8N.h"
#include "SpehsEngine/Core/Log.h"
#include <iostream>
#include <cassert>

namespace codex
{
	namespace device
	{
		Ublox_M8N::Ublox_M8N()
			: transmitPin(gpio::pin_none)
			, receivePin(gpio::pin_none)
			, transmitPinState(gpio::low)
			, receivePinState(gpio::low)
			, readInterval(spehs::time::fromSeconds(1.0f / 9600.0f))
			, stopBitCount(1)
			, parityBitCount(0)
			, receiveState(ReceiveState::detectStreamBoundaries)
		{
		}

		Ublox_M8N::~Ublox_M8N()
		{
		}

		void Ublox_M8N::setPins(const gpio::Pin transmit, const gpio::Pin receive)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			transmitPin = transmit;
			receivePin = receive;
		}

		void Ublox_M8N::onStart()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			transmitPinState = gpio::read(transmitPin);
			receivePinState = gpio::read(receivePin);

			receiveState = ReceiveState::detectStreamBoundaries;
		}

		void Ublox_M8N::update()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
						
			if (receiveState == ReceiveState::detectStreamBoundaries)
			{//Block until able to resume to normal loop
				spehs::log::info("Starting to detect stream boundaries...");

				//TODO: detect read interval

				std::vector<gpio::PinState> history;
				gpio::PinState previousReadState = gpio::read(transmitPin);
				while (previousReadState == gpio::read(transmitPin))
				{
					//Blocks until state change is detected
				}
				spehs::time::Time nextReadTime = spehs::time::now() + readInterval + readInterval / 2;
				spehs::log::info("Initial transmitter pin state detected.");

				int analyzeCount = 0;
				int changingEdgeSamples = 0;
				int timedSamples = 0;
				while (true)
				{
					//Wait until reached next read time or state changes before
					while (true)
					{
						const gpio::PinState readState = gpio::read(transmitPin);
						if (readState != previousReadState)
						{//State changed, synchronize clock at this point
							previousReadState = readState;
							history.push_back(readState);
							nextReadTime = spehs::time::now() + readInterval + readInterval / 2;
							changingEdgeSamples++;
							break;
						}

						if (spehs::time::now() >= nextReadTime)
						{//Reached the next read time mark without the pin state changing
							history.push_back(previousReadState);
							nextReadTime += readInterval;
							timedSamples++;
							break;
						}
					}

					stopBitCount = analyzeCount % 2 ? 1 : 2;
					parityBitCount = analyzeCount / 2 % 2 ? 0 : 1;
					const int sequenceLength = 1/*start bit*/ + 8/*data bits*/ + parityBitCount + stopBitCount;
					static const int minRequiredSequenceCount = 10;//At least this many sequences are required to make credible analyzation
					if (history.size() > sequenceLength * minRequiredSequenceCount)
					{////Analyze...
						/*
						Look for a pattern:
							1 start bit (high)
							8 data bits (x)
							1 parity bit (x)
							1-2 stop bits (low)
						*/
						std::vector<int/*history index*/> potentialStartBits;
						const int endOffset = sequenceLength - 1;
						const int requiredPatternLength = history.size() / sequenceLength - 1;
						for (int i = 0; i + endOffset < history.size(); i++)
						{
							if (history[i] == gpio::PinState::high && history[i + endOffset] == gpio::PinState::low)
								potentialStartBits.push_back(i);
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
						
						if (true)
						{
							spehs::log::info(
								"Analyze #" + std::to_string(++analyzeCount) + " Results: "
								+ std::to_string(history.size()) + " history samples, "
								+ std::to_string(potentialStartBits.size()) + " potential start bits, "
								+ std::to_string(validPatterns.size()) + " valid patterns. "
								+ " Longest sequence pattern: " + std::to_string(longestSequencePattern)
								+ " Stop bit count set to: " + std::to_string(stopBitCount)
								+ " Parity bit count set to: " + std::to_string(parityBitCount)
								+ " Changing edge samples: " + std::to_string(changingEdgeSamples)
								+ " Timed samples: " + std::to_string(timedSamples));
						}
						
						if (spehs::time::now() >= nextReadTime)
						{//Analyze took too long! Missed the next read!
							history.clear();
							previousReadState = gpio::read(transmitPin);
							nextReadTime = std::numeric_limits<spehs::time::Time>::max();//Wait for pin state to change before reading again
							changingEdgeSamples = 0;
							timedSamples = 0;
							spehs::log::info("Analyze took too long! Sample history has been reset.");
						}

						if (validPatterns.size() == 1)
						{//Found exactly one valid pattern! Calculate
							//TODO: wait until ready to receive the start bit
							break;
						}
					}
				}
			}

			if (receiveState == ReceiveState::awaitingStartBit)
			{

			}
			else if (receiveState == ReceiveState::receivingData)
			{

			}
			else if (receiveState == ReceiveState::receivingParityBit)
			{

			}
		}

		void Ublox_M8N::onStop()
		{

		}
	}
}