#include "Codex/Sync/SyncTypeInfo.h"
#include "Codex/Protocol.h"



namespace codex
{
	namespace sync
	{
		void LocalSyncTypeInfo::write(protocol::WriteBuffer& buffer) const
		{
			buffer.write(name);
			buffer.write(typeId);
			buffer.write(version);
		}

		void LocalSyncTypeInfo::read(protocol::ReadBuffer& buffer)
		{
			buffer.read(name);
			buffer.read(typeId);
			buffer.read(version);
		}

		void RemoteSyncTypeInfo::write(protocol::WriteBuffer& buffer) const
		{
			buffer.write(name);
			buffer.write(typeId);
			buffer.write(version);
		}

		void RemoteSyncTypeInfo::read(protocol::ReadBuffer& buffer)
		{
			buffer.read(name);
			buffer.read(typeId);
			buffer.read(version);
		}

		void TypeInfo::write(protocol::WriteBuffer& buffer) const
		{
			buffer.write(local);
			buffer.write(remote);
		}

		void TypeInfo::read(protocol::ReadBuffer& buffer)
		{
			buffer.read(local);
			buffer.read(remote);
		}
	}
}