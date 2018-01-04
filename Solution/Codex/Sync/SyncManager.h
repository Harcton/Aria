#pragma once
#include <functional>
#include <vector>
#include <atomic>
#include <boost/signal.hpp>
#include "Sync/SyncTypeInfo.h"
#include "Sync/SyncEntry.h"
#include "Sync/SyncHandle.h"
#include "SpehsEngine/Core/Time.h"
#include "SpehsEngine/Core/Log.h"

namespace codex
{
	class SocketTCP;
	namespace protocol
	{
		class WriteBuffer;
		class ReadBuffer;
	}
	
	namespace sync
	{
		class Manager
		{
		public:
			enum class PacketType : uint8_t
			{
				invalid,
				types,
				localCreate,
				remoteCreate,
				localUpdate,
				remoteUpdate,
				localRemove,//Waits for the remote type to send remove packet before actually removing
				remoteRemove
			};
			enum class TypeCompatibility : uint8_t
			{
				unknown,
				compatible,
				incompatible,
			};

		public:

			Manager(SocketTCP& socket);
			~Manager();

			template<typename LocalSyncType, typename RemoteSyncType>
			void registerType()
			{
				static_assert(!std::is_same<IType, LocalSyncType>::value, "Cannot register IType as a synchronizable type!");
				static_assert(std::is_base_of<IType, LocalSyncType>::value, "Cannot register a SyncType that is not derived from IType!");
				static_assert(!std::is_same<IType, RemoteSyncType>::value, "Cannot register IType as a synchronizable type!");
				static_assert(std::is_base_of<IType, RemoteSyncType>::value, "Cannot register a SyncType that is not derived from IType!");
				if (initialized)
				{
					spehs::log::error("codex::sync::Manager cannot register new types once initialized!");
					return;
				}

				//Register type data
				registeredTypes.push_back(SyncTypePairInfo());
				registeredTypes.back().local.name = LocalSyncType::getSyncTypeName();
				registeredTypes.back().remote.name = RemoteSyncType::getSyncTypeName();
				registeredTypes.back().local.typeId = LocalSyncType::getSyncTypeId();
				registeredTypes.back().remote.typeId = RemoteSyncType::getSyncTypeId();
				registeredTypes.back().local.version = LocalSyncType::getSyncTypeVersion();
				registeredTypes.back().remote.version = RemoteSyncType::getSyncTypeVersion();
				registeredTypes.back().remote.constructor = []() {return new RemoteSyncType(); };
			}

			//Connect and initialize
			bool initialize();
			bool isInitialized() const;

			void update(const spehs::time::Time& deltaTime);

			/* Allocates a new instance of the provided type. Returns a handle. If no handle copies are made, the instance will be deallocated during the next update. */
			template<typename LocalSyncType, typename ... Args>
			Handle<LocalSyncType> create(Args ... constructorParams)
			{
				static_assert(!std::is_same<IType, LocalSyncType>::value, "Cannot create LocalSyncType as a synchronizable type!");
				static_assert(std::is_base_of<IType, LocalSyncType>::value, "Cannot create a SyncType that is not derived from IType!");
				LocalSyncTypeInfo* typeInfo = findLocalTypeInfo(LocalSyncType::getSyncTypeId());
				if (!typeInfo)
				{
					spehs::log::info("codex::sync::Manager::create: Cannot create a type that is not registered.");
					return Handle<LocalSyncType>();
				}
				localEntries.push_back(new LocalEntry(*this, new LocalSyncType(constructorParams...), nextLocalEntryId++, *typeInfo));
				return Handle<LocalSyncType>(*localEntries.back());
			}

			/* Fires when a remote type is created. */
			//boost::signal<void(IType&)> remoteTypeCreatedSignal;

		private:
			bool receiveHandler(protocol::ReadBuffer& buffer);
			
			LocalEntry* findLocalEntry(const AbstractEntry::Id id);
			RemoteEntry* findRemoteEntry(const AbstractEntry::Id id);
			LocalSyncTypeInfo* findLocalTypeInfo(const IType::SyncTypeIdType& typeId);
			LocalSyncTypeInfo* findLocalTypeInfo(const RemoteSyncTypeInfo& remoteCounterpart);
			RemoteSyncTypeInfo* findRemoteTypeInfo(const IType::SyncTypeIdType& typeId);
			RemoteSyncTypeInfo* findRemoteTypeInfo(const LocalSyncTypeInfo& localCounterpart);
									
			bool initialized;
			SocketTCP& socket;
			LocalEntry::Id nextLocalEntryId;
			std::vector<SyncTypePairInfo> registeredTypes;
			std::vector<LocalEntry*> localEntries;
			std::vector<RemoteEntry*> remoteEntries;

			/* Sync managers are connected to each other. */
			class RemoteManager
			{
			public:
				RemoteManager()
					: typeCompatibility(TypeCompatibility::unknown)
					, typeCompatibilityResponse(TypeCompatibility::unknown)
				{

				}
				std::atomic<TypeCompatibility> typeCompatibility;//My evaluation
				std::atomic<TypeCompatibility> typeCompatibilityResponse;//Remote evaluation
			} remoteManager;
		};
	}
}