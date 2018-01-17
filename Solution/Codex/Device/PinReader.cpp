#include <cstring>
#include "Codex/Device/PinReader.h"
#include "Codex/Protocol.h"

namespace codex
{
	namespace device
	{
		void PinReaderHistoryEntry::write(protocol::WriteBuffer& buffer) const
		{
			buffer.write(state);
			buffer.write(time);
		}

		void PinReaderHistoryEntry::read(protocol::ReadBuffer& buffer)
		{
			buffer.read(state);
			buffer.read(time);
		}

		void PinReaderHistory::write(protocol::WriteBuffer& buffer) const
		{
			const size_t count = size();//TODO: buffer.write(std::vector<...>)
			buffer.write(count);
			for (size_t i = 0; i < count; i++)
				buffer.write(at(i));
		}

		void PinReaderHistory::read(protocol::ReadBuffer& buffer)
		{
			size_t count;
			buffer.read(count);//TODO: buffer.read(std::vector<...>)
			resize(count);
			for (size_t i = 0; i < count; i++)
				buffer.read(at(i));
		}



		PinReaderGhost::PinReaderGhost()
		{

		}

		PinReaderGhost::~PinReaderGhost()
		{

		}

		void PinReaderGhost::syncCreate(protocol::WriteBuffer& buffer)
		{

		}

		void PinReaderGhost::syncCreate(protocol::ReadBuffer& buffer)
		{

		}

		void PinReaderGhost::syncRemove(protocol::WriteBuffer& buffer)
		{

		}

		void PinReaderGhost::syncRemove(protocol::ReadBuffer& buffer)
		{

		}

		bool PinReaderGhost::syncUpdate(const spehs::time::Time deltaTime)
		{
			return requestUpdate;
		}

		void PinReaderGhost::syncUpdate(protocol::WriteBuffer& buffer)
		{
			buffer.write(active);
			buffer.write(pin);
			requestUpdate = false;
		}

		void PinReaderGhost::syncUpdate(protocol::ReadBuffer& buffer)
		{
			bool clearHistory;
			PinReaderHistory newHistory;
			buffer.read(clearHistory);
			if (clearHistory)
				history.clear();
			buffer.read(newHistory);
			history.insert(history.end(), newHistory.begin(), newHistory.end());
		}

		void PinReaderGhost::setPin(const gpio::Pin newPin)
		{
			if (pin != newPin)
			{
				pin = newPin;
				requestUpdate = true;
			}
		}

		void PinReaderGhost::setActive(const bool isActive)
		{
			if (active != isActive)
			{
				active = isActive;
				requestUpdate = true;
			}
		}
		
		void PinReaderGhost::clearHistory()
		{
			history.clear();
		}

		

		PinReaderShell::PinReaderShell()
			: pin(gpio::Pin::pin_none)
			, clearHistory(false)
			, active(false)
			, lastReadTime(0)
		{

		}

		PinReaderShell::~PinReaderShell()
		{

		}

		void PinReaderShell::setActive(const bool isActive)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			if (active != isActive)
			{
				active = isActive;
				if (active)
				{
					start();
				}
				else
				{
					stop();
				}
			}
		}

		void PinReaderShell::setPin(const gpio::Pin newPin)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			if (pin != newPin)
			{
				spehs::log::info("Setting pin reader pin to: " + std::to_string(gpio::getPinEnumAsNumber(newPin)));
				pin = newPin;
				clearHistory = true;
				history.clear();
			}
		}

		void PinReaderShell::syncCreate(protocol::WriteBuffer& buffer)
		{

		}

		void PinReaderShell::syncCreate(protocol::ReadBuffer& buffer)
		{

		}

		void PinReaderShell::syncRemove(protocol::WriteBuffer& buffer)
		{

		}

		void PinReaderShell::syncRemove(protocol::ReadBuffer& buffer)
		{
			stop();
			while (isRunning()) {}
		}

		bool PinReaderShell::syncUpdate(const spehs::time::Time deltaTime)
		{
			if (history.size() * sizeof(spehs::time::Time) > 1400)
				return true;//Default MTU is around 1500, preferably keep it below that
			if (spehs::time::now() - lastReadTime >= spehs::time::fromSeconds(1.0f))
				return true;
			return false;
		}

		void PinReaderShell::syncUpdate(protocol::WriteBuffer& buffer)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			buffer.write(clearHistory);
			clearHistory = false;
			buffer.write(history);
			history.clear();
		}

		void PinReaderShell::syncUpdate(protocol::ReadBuffer& buffer)
		{
			bool _active;
			buffer.read(_active);
			setActive(_active);
			gpio::Pin _pin;
			buffer.read(_pin);
			setPin(_pin);
		}

		void PinReaderShell::getHistory(PinReaderHistory& deposit) const
		{
			deposit = history;
		}

		void PinReaderShell::onStart()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			history.clear();
		}

		void PinReaderShell::update()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			const gpio::PinState state = gpio::read(pin);
			if (history.empty() || state != history.back().state)
			{
				history.push_back(PinReaderHistoryEntry(state, spehs::time::now()));
			}
		}

		void PinReaderShell::onStop()
		{
			
		}
	}
}