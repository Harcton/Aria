#include "Codex/Sync/SyncTypeInfo.h"
#include "Codex/Protocol.h"



namespace codex
{
	namespace sync
	{
		void AbstractSyncTypeInfo::write(protocol::WriteBuffer& buffer) const
		{
			buffer.write(name);
			buffer.write(typeId);
			buffer.write(version);
		}

		void AbstractSyncTypeInfo::read(protocol::ReadBuffer& buffer)
		{
			buffer.read(name);
			buffer.read(typeId);
			buffer.read(version);
		}

		void SyncTypePairInfo::write(protocol::WriteBuffer& buffer) const
		{
			buffer.write(local);
			buffer.write(remote);
		}

		void SyncTypePairInfo::read(protocol::ReadBuffer& buffer)
		{
			buffer.read(local);
			buffer.read(remote);
		}
	}
}