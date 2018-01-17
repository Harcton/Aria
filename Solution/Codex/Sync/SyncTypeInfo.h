#pragma once
#include <functional>
#include "Codex/Sync/ISyncType.h"



namespace codex
{
	namespace sync
	{
		//struct AbstractSyncTypeInfo
		//{
		//	void write(protocol::WriteBuffer& buffer) const;
		//	void read(protocol::ReadBuffer& buffer);

		//	IType::SyncTypeIdType typeId;
		//	std::string name;
		//	IType::SyncTypeVersionType version;
		//};

		struct LocalSyncTypeInfo
		{
			void write(protocol::WriteBuffer& buffer) const;
			void read(protocol::ReadBuffer& buffer);
			std::string name;
			IType::SyncTypeIdType typeId;
			IType::SyncTypeVersionType version;
			std::function<IType*()> constructor;
		};

		struct RemoteSyncTypeInfo
		{
			void write(protocol::WriteBuffer& buffer) const;
			void read(protocol::ReadBuffer& buffer);
			std::string name;
			IType::SyncTypeIdType typeId;
			IType::SyncTypeVersionType version;
		};

		struct TypeInfo
		{
			void write(protocol::WriteBuffer& buffer) const;
			void read(protocol::ReadBuffer& buffer);
			LocalSyncTypeInfo local;
			RemoteSyncTypeInfo remote;
		};
	}
}