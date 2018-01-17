#pragma once
#include <stdint.h>
#include <vector>
#include "SpehsEngine/Core/Time.h"
#include "SyncTypeInfo.h"



namespace codex
{
	namespace sync
	{
		class AbstractHandle;
		class Manager;
		class IType;
		
		/*
			Local instances are managed through entries.
		*/
		class Entry
		{
		public:
			typedef uint16_t Id;
		public:

			sync::Manager& manager;
			sync::IType* ptr;
			const Id id;

			bool createSent;
			bool removeSent;
			bool removeReceived;
			bool createReceived;
			const bool locallyInstantiated;//True if instantiated locally, lifetime depends on handles.

			const TypeInfo& typeInfo;
			std::vector<AbstractHandle*> handles;
			spehs::time::Time interval;
			spehs::time::Time timer;

		private:
			friend class Manager;
			Entry(Manager& _manager, IType* _passData, const Id _id, const TypeInfo& _typeInfo, const bool locallyInstantiated);
			virtual ~Entry();
		};
	}
}