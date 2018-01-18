#include "SyncHandle.h"
#include "Codex/Sync/SyncHandle.h"
#include "Codex/Sync/SyncEntry.h"
#include "Codex/Sync/SyncManager.h"
#include "Codex/SocketTCP.h"
#include "Codex/Protocol.h"



namespace codex
{
	namespace sync
	{
		AbstractHandle::AbstractHandle(Entry* _entry)
			: entry(_entry)
		{

		}

		void AbstractHandle::setLocalUpdateInterval(const spehs::time::Time& interval)
		{
			if (entry->interval != interval)
			{
				entry->interval = interval;
			}
		}

		void AbstractHandle::setRemoteUpdateInterval(const spehs::time::Time& interval)
		{
			entry->sendRemoteUpdateInterval = true;
			entry->requestedRemoteUpdateInterval = interval;
		}
	}
}