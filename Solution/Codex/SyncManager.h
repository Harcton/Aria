#pragma once
#include "SyncType.h"
#include "CodexTime.h"
#include "SocketTCP.h"
#include "Log.h"

namespace codex
{
	class SocketTCP;
	namespace protocol
	{
		class WriteBuffer;
		class ReadBuffer;
	}

	class SyncManager
	{
	public:

		struct Entry
		{
			typedef uint16_t Id;
			Entry(SyncType& ref) : ptr(&ref) {}

			SyncType* ptr;
			Id id = 0;
			time::TimeType interval;
			time::TimeType timer;
		};

	public:

		SyncManager(SocketTCP& socketRef);
		~SyncManager();

		template<typename T>
		void registerType()
		{
			static_assert(!std::is_same<SyncType, T>::value, "Cannot register SyncType as a synchronizable type!");
			static_assert(std::is_base_of<SyncType, T>::value, "Cannot register a type that is not derived from SyncType!");
			if (initialized)
			{
				codex::log::error("SyncManager cannot register new types once initialized!");
				return;
			}

			//TODO: register type data
		}

		bool initialize();
		void update(const time::TimeType& deltaTime);
		void addEntry(SyncType& reference, const time::TimeType& syncInterval);
		void removeEntry(SyncType& reference);
		void setSyncInterval(SyncType& reference, const time::TimeType& syncInterval);
		
	private:
		SyncType* allocate(const SyncType::TypeId typeId);
		void create(protocol::ReadBuffer& buffer);
		void remove(protocol::ReadBuffer& buffer);
		
		bool initialized;
		SocketTCP& socket;
		std::vector<Entry> entriesToAdd;
		std::vector<Entry> entries;
		Entry::Id nextGeneratedId;

		/* Sync managers are connected to each other. */
		class RemoteSyncManager
		{

		} remoteSyncManager;
	};
}