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

	/*
		Ghost sync types are created and controlled outside the sync manager.
		They are then connected to the sync manager, after which they start to exist on a remote sync manager as the ShellSyncType counterpart.
		The remote counterpart is contained inside the remote sync manager.
	*/
	class GhostSyncType
	{
	public:
		typedef uint32_t GhostSyncTypeVersionType;
	public:
		virtual ~GhostSyncType() {}
		/* By returning true, the ghost sync type indicates that it is ready to send an update packet. */
		virtual bool ghostSyncTypeUpdate(const time::TimeType& deltaTime) { return false; }
		/* Called when entry is added to a sync manager's synced entries. */
		virtual void onSyncManagerAddEntry(SyncManager* syncManager) {}
		/* Called when the entry is removed from the sync manager's synced entries. */
		virtual void onSyncManagerRemoveEntry(SyncManager* syncManager) {}
		virtual void syncCreateGhost(protocol::WriteBuffer& buffer) {}
		virtual void syncCreateShell(protocol::ReadBuffer& buffer) {}
		virtual void syncUpdateGhost(protocol::WriteBuffer& buffer) {}
		virtual void syncUpdateShell(protocol::ReadBuffer& buffer) {}
		virtual void syncRemoveGhost(protocol::WriteBuffer& buffer) {}
		virtual void syncRemoveShell(protocol::ReadBuffer& buffer) {}
	};

	/*
		Shell sync types exist inside the SyncManager's RemoteSyncManager's container.
	*/
	class ShellSyncType
	{
	public:
		typedef uint32_t ShellSyncTypeVersionType;
	public:
		virtual ~ShellSyncType() {}
		/* By returning true, the shell sync type indicates that it is ready to send an update packet. */
		virtual bool shellSyncTypeUpdate(const time::TimeType& deltaTime) { return false; }
		virtual void syncCreateGhost(protocol::ReadBuffer& buffer) {}
		virtual void syncCreateShell(protocol::WriteBuffer& buffer) {}
		virtual void syncUpdateGhost(protocol::ReadBuffer& buffer) {}
		virtual void syncUpdateShell(protocol::WriteBuffer& buffer) {}
		virtual void syncRemoveGhost(protocol::ReadBuffer& buffer) {}
		virtual void syncRemoveShell(protocol::WriteBuffer& buffer) {}
	};
}

#define SYNCTYPE_DECL(p_TypeType, p_MyClass, p_Version) \
public: \
static const std::string& get##p_TypeType##SyncTypeName() \
{ \
	static const std::string name = #p_MyClass; \
	return name; \
} \
static const p_TypeType##SyncTypeVersionType get##p_TypeType##SyncTypeVersion() \
{ \
	static const p_TypeType##SyncTypeVersionType version = p_Version; \
	return version; \
}
#define GHOST_SYNCTYPE_DECL(p_MyClass, p_Version) SYNCTYPE_DECL(Ghost, p_MyClass, p_Version)
#define SHELL_SYNCTYPE_DECL(p_MyClass, p_Version) SYNCTYPE_DECL(Shell, p_MyClass, p_Version)