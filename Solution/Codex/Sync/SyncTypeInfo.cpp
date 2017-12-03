#include "Codex/Sync/SyncTypeInfo.h"
#include "Codex/Protocol.h"



namespace codex
{
	namespace sync
	{
		void SyncTypePairInfo::write(protocol::WriteBuffer& buffer)
		{
			buffer.write(local.name);
			buffer.write(local.typeId);
			buffer.write(local.version);
			buffer.write(remote.name);
			buffer.write(remote.typeId);
			buffer.write(remote.version);
		}

		void SyncTypePairInfo::read(protocol::ReadBuffer& buffer)
		{
			buffer.read(local.name);
			buffer.read(local.typeId);
			buffer.read(local.version);
			buffer.read(remote.name);
			buffer.read(remote.typeId);
			buffer.read(remote.version);
		}
	}
}