#include "SyncEntry.h"
#include "SpehsEngine/Core/Log.h"



namespace codex
{
	namespace sync
	{
		Entry::Entry(Manager& _manager, IType* _passData, const Id _id, const TypeInfo& _typeInfo, const bool _locallyInstantiated)
			: manager(_manager)
			, ptr(_passData)
			, id(_id)
			, createSent(false)
			, removeSent(false)
			, removeReceived(false)
			, createReceived(false)
			, locallyInstantiated(_locallyInstantiated)
			, handles()
			, interval(0)
			, timer(0)
			, typeInfo(_typeInfo)
		{
			SPEHS_ASSERT(_passData);
		}

		Entry::~Entry()
		{
			delete ptr;
		}
	}
}