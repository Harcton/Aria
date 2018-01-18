#pragma once
#include <mutex>
#include <vector>
#include <thread>
#include "SpehsEngine/Core/Time.h"
#include "Codex/Device/ThreadedDevice.h"
#include "Codex/GPIO.h"
#include "Codex/Sync/ISyncType.h"

namespace codex
{
	namespace protocol
	{
		class WriteBuffer;
		class ReadBuffer;
	}

	namespace device
	{
		/*
			Stores the begin state and time steps between each pin state flip.
		*/
		struct PinReaderHistoryEntry
		{
			PinReaderHistoryEntry() = default;
			PinReaderHistoryEntry(gpio::PinState s, const spehs::time::Time t) : state(s), time(t) {}
			void write(protocol::WriteBuffer& buffer) const;
			void read(protocol::ReadBuffer& buffer);
			gpio::PinState state;
			spehs::time::Time time;
		};
		class PinReaderHistory : public std::vector<PinReaderHistoryEntry>
		{
		public:
			void write(protocol::WriteBuffer& buffer) const;
			void read(protocol::ReadBuffer& buffer);
			gpio::PinState stateAtTime(const spehs::time::Time& time) const;
		};

		class PinReaderGhost : public sync::IType
		{
			CODEX_SYNC_TYPE_DECL(PinReaderGhost, 0)
		public:
			PinReaderGhost();
			~PinReaderGhost();

			//Sync type
			void syncCreate(protocol::WriteBuffer& buffer) override;
			void syncCreate(protocol::ReadBuffer& buffer) override;
			void syncRemove(protocol::WriteBuffer& buffer) override;
			void syncRemove(protocol::ReadBuffer& buffer) override;
			bool syncUpdate(const spehs::time::Time deltaTime) override;
			void syncUpdate(protocol::WriteBuffer& buffer) override;
			void syncUpdate(protocol::ReadBuffer& buffer) override;

			void setPin(const gpio::Pin pin);
			void setActive(const bool isActive);
			void clearHistory();

			const PinReaderHistory& getHistory() const { return history; }
			const gpio::Pin getPin() const { return pin; }

			std::string name;

		private:
			gpio::Pin pin;
			PinReaderHistory history;
			bool active;
			bool requestUpdate;
		};

		class PinReaderShell final : public ThreadedDevice, public sync::IType
		{
			CODEX_SYNC_TYPE_DECL(PinReaderShell, 0)
		public:
			PinReaderShell();
			~PinReaderShell();

			void setActive(const bool isActive);
			void setPin(const gpio::Pin pin);
			void getHistory(PinReaderHistory& deposit) const;

			//Sync type
			void syncCreate(protocol::WriteBuffer& buffer) override;
			void syncCreate(protocol::ReadBuffer& buffer) override;
			void syncRemove(protocol::WriteBuffer& buffer) override;
			void syncRemove(protocol::ReadBuffer& buffer) override;
			bool syncUpdate(const spehs::time::Time deltaTime) override;
			void syncUpdate(protocol::WriteBuffer& buffer) override;
			void syncUpdate(protocol::ReadBuffer& buffer) override;


		private:
			//Threaded device
			void onStart() override;
			void update() override;
			void onStop() override;

			std::recursive_mutex mutex;
			gpio::Pin pin;
			bool clearHistory;
			bool active;
			PinReaderHistory history;
		};
	}
}