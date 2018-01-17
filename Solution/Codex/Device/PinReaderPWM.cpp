#include <cstring>
#include "Codex/Device/PinReaderPWM.h"
#include "Codex/Protocol.h"

namespace codex
{
	namespace device
	{
		PinReaderPWMGhost::PinReaderPWMGhost()
		{

		}

		PinReaderPWMGhost::~PinReaderPWMGhost()
		{

		}

		void PinReaderPWMGhost::syncCreate(protocol::WriteBuffer& buffer)
		{

		}

		void PinReaderPWMGhost::syncCreate(protocol::ReadBuffer& buffer)
		{

		}

		void PinReaderPWMGhost::syncRemove(protocol::WriteBuffer& buffer)
		{

		}

		void PinReaderPWMGhost::syncRemove(protocol::ReadBuffer& buffer)
		{

		}

		bool PinReaderPWMGhost::syncUpdate(const spehs::time::Time deltaTime)
		{
			return requestUpdate;
		}

		void PinReaderPWMGhost::syncUpdate(protocol::WriteBuffer& buffer)
		{
			buffer.write(active);
			buffer.write(pin);
			requestUpdate = false;
		}

		void PinReaderPWMGhost::syncUpdate(protocol::ReadBuffer& buffer)
		{
			bool clearHistory;
			PWMHistory newHistory;
			buffer.read(clearHistory);
			if (clearHistory)
				history.clear();
			buffer.read(newHistory);
			const size_t oldHistoryOffset = history.size();
			history.resize(history.size() + newHistory.size());
			if (newHistory.size() > 0)
			{
				memcpy(&history[oldHistoryOffset], newHistory.data(), sizeof(newHistory.front()) * newHistory.size());
			}
		}

		void PinReaderPWMGhost::setPin(const gpio::Pin newPin)
		{
			if (pin != newPin)
			{
				pin = newPin;
				requestUpdate = true;
			}
		}

		void PinReaderPWMGhost::setActive(const bool isActive)
		{
			if (active != isActive)
			{
				active = isActive;
				requestUpdate = true;
			}
		}
		
		void PinReaderPWMGhost::clearHistory()
		{
			history.clear();
		}

		

		PinReaderPWMShell::PinReaderPWMShell()
			: pin(gpio::Pin::pin_none)
			, clearHistory(false)
			, active(false)
			, lastReadTime(0)
		{

		}

		PinReaderPWMShell::~PinReaderPWMShell()
		{

		}

		void PinReaderPWMShell::setActive(const bool isActive)
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

		void PinReaderPWMShell::setPin(const gpio::Pin newPin)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			if (pin != newPin)
			{
				spehs::log::info("Setting pin reader pin to: " + std::to_string(gpio::getPinEnumAsNumber(newPin)));
				pin = newPin;
				clearHistory = true;
				pwmHistory.clear();
				pinReaderHistory.clear();
				lastReadTime = spehs::time::now();
			}
		}

		void PinReaderPWMShell::syncCreate(protocol::WriteBuffer& buffer)
		{

		}

		void PinReaderPWMShell::syncCreate(protocol::ReadBuffer& buffer)
		{

		}

		void PinReaderPWMShell::syncRemove(protocol::WriteBuffer& buffer)
		{

		}

		void PinReaderPWMShell::syncRemove(protocol::ReadBuffer& buffer)
		{
			stop();
			while (isRunning()) {}
		}

		bool PinReaderPWMShell::syncUpdate(const spehs::time::Time deltaTime)
		{
			if (pwmHistory.size() * sizeof(spehs::time::Time) > 1400)
				return true;//Default MTU is around 1500, preferably keep it below that
			if (spehs::time::now() - lastReadTime >= spehs::time::fromSeconds(1.0f))
				return true;
			return false;
		}

		void PinReaderPWMShell::syncUpdate(protocol::WriteBuffer& buffer)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			buffer.write(clearHistory);
			clearHistory = false;
			const size_t historySize = pwmHistory.size();
			buffer.write(historySize);
			for (size_t i = 0; i < historySize; i++)
				buffer.write(pwmHistory[i]);
			pwmHistory.clear();
		}

		void PinReaderPWMShell::syncUpdate(protocol::ReadBuffer& buffer)
		{
			bool _active;
			buffer.read(_active);
			setActive(_active);
			gpio::Pin _pin;
			buffer.read(_pin);
			setPin(_pin);
		}

		void PinReaderPWMShell::getHistory(PWMHistory& deposit) const
		{
			deposit = pwmHistory;
		}

		void PinReaderPWMShell::onStart()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			pwmHistory.clear();
			pinReaderHistory.clear();
			lastReadTime = spehs::time::now();
		}

		void PinReaderPWMShell::update()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			const gpio::PinState state = gpio::read(pin);
			if (pinReaderHistory.empty() || state != pinReaderHistory.back().state)
			{
				pinReaderHistory.push_back(PinReaderHistoryEntry(state, spehs::time::now()));
			}
		}

		void PinReaderPWMShell::onStop()
		{
			
		}
	}
}