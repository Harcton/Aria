#pragma once
#include <stdint.h>
#include <string>
#include "SpehsEngine/Core/Time.h"

namespace codex
{
	namespace protocol
	{
		class WriteBuffer;
		class ReadBuffer;
	}

	namespace sync
	{
		class IType
		{
		public:
			typedef uint32_t SyncTypeVersionType;
			typedef std::string SyncTypeIdType;
		public:

			virtual ~IType() {}

			/* Must implement the following static methods. Use macro below to do this the easy way. */
			//static const std::string& getSyncTypeName();
			//static const SyncTypeVersionType getSyncTypeVersion();

			/* By returning true, the sync type indicates that it is ready to write an update packet. */
			virtual bool syncUpdate(const spehs::time::Time deltaTime) { return false; }

			/* Sync packets */
			virtual void syncCreate(protocol::WriteBuffer& buffer) {}
			virtual void syncCreate(protocol::ReadBuffer& buffer) {}
			virtual void syncUpdate(protocol::WriteBuffer& buffer) {}
			virtual void syncUpdate(protocol::ReadBuffer& buffer) {}
			virtual void syncRemove(protocol::WriteBuffer& buffer) {}
			virtual void syncRemove(protocol::ReadBuffer& buffer) {}

		};
	}
}

#define CODEX_SYNC_TYPE_DECL(p_Name, p_Version) \
public: \
static const std::string& getSyncTypeName() \
{ \
	static const std::string name(#p_Name); \
	return name; \
} \
static const SyncTypeIdType& getSyncTypeId() \
{ \
	static const SyncTypeIdType id = #p_Name; \
	return id; \
} \
static const SyncTypeVersionType getSyncTypeVersion() \
{ \
	static const SyncTypeVersionType version = p_Version; \
	return version; \
}