#include "SyncManager.h"
#include "CodexAssert.h"



namespace codex
{
	SyncManager::SyncManager(SocketTCP& socketRef)
		: initialized(false)
		, socket(socketRef)
		, nextGeneratedId(1)
	{

	}

	SyncManager::~SyncManager()
	{
		//Sync manager is going out of scope -> severe links to entried instances
		for (size_t i = 0; i < entriesToAdd.size(); i++)
		{
			CODEX_ASSERT(entriesToAdd[i].ptr->syncManager == this);
			entriesToAdd[i].ptr->syncManager = nullptr;
		}
		for (size_t i = 0; i < entries.size(); i++)
		{
			CODEX_ASSERT(entries[i].ptr->syncManager == this);
			entries[i].ptr->syncManager = nullptr;
		}
	}

	bool SyncManager::initialize()
	{
		if (initialized)
		{
			codex::log::error("SyncManager is already initialized!");
			return false;
		}

		return true;
	}

	void SyncManager::update(const time::TimeType& deltaTime)
	{
		if (entriesToAdd.size() > 0)
		{
			protocol::WriteBuffer buffer;
			for (size_t i = 0; i < entriesToAdd.size(); i++)
			{
				//TODO: Sync creation
				entries.push_back(entriesToAdd[i]);
			}
			entriesToAdd.clear();
		}

		for (size_t i = 0; i < entries.size(); i++)
		{
			entries[i].timer += deltaTime;
			if (entries[i].timer >= entries[i].interval)
			{
				entries[i].timer -= entries[i].interval;
				//Send sync data
			}
		}
	}

	void SyncManager::addEntry(SyncType& reference, const time::TimeType& syncInterval)
	{
		//Check that entry is not in the to be added queue
		for (size_t i = 0; i < entriesToAdd.size(); i++)
		{
			if (entriesToAdd[i].ptr == &reference)
			{
				log::error("Entry already added!");
				return;
			}
		}
		//Check that entry is not in the added entries
		for (size_t i = 0; i < entries.size(); i++)
		{
			if (entries[i].ptr == &reference)
			{
				log::error("Entry already added!");
				return;
			}
		}

		entriesToAdd.push_back(Entry(reference));
		entriesToAdd.back().interval = syncInterval;
		entriesToAdd.back().timer = syncInterval;
		entriesToAdd.back().id = nextGeneratedId++;
		reference.syncManager = this;
	}

	void SyncManager::removeEntry(SyncType& reference)
	{
		//Search entries to add
		for (size_t i = 0; i < entriesToAdd.size(); i++)
		{
			if (entriesToAdd[i].ptr == &reference)
			{
				//TODO: sync removal
				entriesToAdd.erase(entriesToAdd.begin() + i);
				return;
			}
		}
		//Search entries
		for (size_t i = 0; i < entries.size(); i++)
		{
			if (entries[i].ptr == &reference)
			{
				//TODO: sync removal
				entries.erase(entries.begin() + i);
				return;
			}
		}
		log::warning("Specified entry not found from among the registered entries!");
	}

	void SyncManager::setSyncInterval(SyncType& reference, const time::TimeType& syncInterval)
	{
		for (size_t i = 0; i < entriesToAdd.size(); i++)
		{
			if (entriesToAdd[i].ptr == &reference)
			{
				entriesToAdd[i].interval = syncInterval;
				if (entriesToAdd[i].timer > syncInterval)
					entriesToAdd[i].timer = syncInterval;
				return;
			}
		}
		for (size_t i = 0; i < entries.size(); i++)
		{
			if (entries[i].ptr == &reference)
			{
				entries[i].interval = syncInterval;
				if (entries[i].timer > syncInterval)
					entries[i].timer = syncInterval;
				return;
			}
		}
		log::warning("Specified entry not found from among the registered entries!");
	}

	SyncType* SyncManager::allocate(const SyncType::TypeId typeId)
	{
		//TODO
		return nullptr;
	}
	void SyncManager::create(protocol::ReadBuffer& buffer)
	{
		SyncType::TypeId typeId;
		typeId.read(buffer);
	}
	void SyncManager::remove(protocol::ReadBuffer& buffer)
	{
		Entry::Id entryId;
		buffer.read((uint16_t&)entryId);
		//TODO:
	}
}