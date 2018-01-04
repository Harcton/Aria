#include "SyncEntry.h"
#include "SpehsEngine/Core/Log.h"



namespace codex
{
	namespace sync
	{
		AbstractEntry::AbstractEntry(Manager& _manager, IType* _passData, const Id _id)
			: manager(_manager)
			, ptr(_passData)
			, id(_id)
			, createSent(false)
			, removeSent(false)
			, removeReceived(false)
		{
			SPEHS_ASSERT(_passData);
		}

		AbstractEntry::~AbstractEntry()
		{
			delete ptr;
		}
		
		LocalEntry::LocalEntry(Manager& _manager, IType* _passData, const Id _id, const LocalSyncTypeInfo& _typeInfo)
			: AbstractEntry(_manager, _passData, _id)
			, typeInfo(_typeInfo)
			, handles()
			, interval(0)
			, timer(0)
			, createReceived(false)
		{

		}

		LocalEntry::~LocalEntry()
		{

		}

		RemoteEntry::RemoteEntry(Manager& _manager, IType* _passData, const Id _id, const RemoteSyncTypeInfo& _typeInfo)
			: AbstractEntry(_manager, _passData, _id)
			, typeInfo(_typeInfo)
		{

		}

		RemoteEntry::~RemoteEntry()
		{

		}
	}
}