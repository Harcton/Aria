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

		

		class AbstractEntry
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

		protected:
			friend class Manager;
			AbstractEntry(Manager& _manager, IType* _passData, const Id _id);
			virtual ~AbstractEntry();
		};

		class LocalEntry : public AbstractEntry
		{
		public:			

			const LocalSyncTypeInfo& typeInfo;
			std::vector<AbstractHandle*> handles;
			spehs::time::Time interval;
			spehs::time::Time timer;
			bool createReceived;

		private:
			friend class Manager;
			LocalEntry(Manager& _manager, IType* _passData, const Id _id, const LocalSyncTypeInfo& _typeInfo);
			~LocalEntry();
		};
		
		class RemoteEntry : public AbstractEntry
		{
		public:
			typedef uint16_t Id;
		public:

			const RemoteSyncTypeInfo& typeInfo;
			
		private:
			friend class Manager;
			RemoteEntry(Manager& _manager, IType* _passData, const Id _id, const RemoteSyncTypeInfo& _typeInfo);
			~RemoteEntry();
		};
	}
}