#pragma once
#include <functional>
#include "Codex/Sync/ISyncType.h"



namespace codex
{
	namespace sync
	{
		struct AbstractSyncTypeInfo
		{
			IType::SyncTypeIdType typeId;
			std::string name;
			IType::SyncTypeVersionType version;
		};

		struct LocalSyncTypeInfo : public AbstractSyncTypeInfo
		{

		};

		struct RemoteSyncTypeInfo : public AbstractSyncTypeInfo
		{
			std::function<IType*()> constructor;
		};

		struct SyncTypePairInfo
		{
			void write(protocol::WriteBuffer& buffer);
			void read(protocol::ReadBuffer& buffer);
			LocalSyncTypeInfo local;
			RemoteSyncTypeInfo remote;
		};
	}
}