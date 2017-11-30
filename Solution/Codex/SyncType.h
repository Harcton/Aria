#pragma once
#include <stdint.h>
#include "CodexTime.h"
#include "CodexAssert.h"

namespace codex
{
	namespace protocol
	{
		class WriteBuffer;
		class ReadBuffer;
	}
	class SyncManager;

	class ISyncType
	{
	public:
		typedef uint32_t SyncTypeVersionType;
	public:

		/* Must implement the following static methods. Use macro below to do this the easy way. */
		//static const std::string& getSyncTypeName();
		//static const SyncTypeVersionType getSyncTypeVersion();

		/* By returning true, the sync type indicates that it is ready to write an update packet. */
		virtual bool syncUpdate(const time::TimeType& deltaTime) { return false; }

		/* Sync packets */
		virtual void syncCreate(protocol::WriteBuffer& buffer) {}
		virtual void syncCreate(protocol::ReadBuffer& buffer) {}
		virtual void syncUpdate(protocol::WriteBuffer& buffer) {}
		virtual void syncUpdate(protocol::ReadBuffer& buffer) {}
		virtual void syncRemove(protocol::WriteBuffer& buffer) {}
		virtual void syncRemove(protocol::ReadBuffer& buffer) {}
	};
}

#define CODEX_SYNCTYPE_DECL(p_Name, p_Version) \
public: \
static const std::string& getSyncTypeName() \
{ \
	static const std::string name(#p_Name); \
	return name; \
} \
static const SyncTypeVersionType getSyncTypeVersion() \
{ \
	static const SyncTypeVersionType version = p_Version; \
	return version; \
}