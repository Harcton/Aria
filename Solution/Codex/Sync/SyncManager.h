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
				create,
				createResponse,
				update,
				remove,
				updateInterval,
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

			/*
				Register a local type.
				The remote type implementation is unknown, but its name, id and version must be known.
				The remote type must have a default constructor.
			*/
			template<typename LocalSyncType>
			void registerType(const std::string& remoteTypeName, const IType::SyncTypeIdType& remoteTypeId, const size_t remoteTypeVersion)
			{
				static_assert(!std::is_same<IType, LocalSyncType>::value, "Cannot register IType as a synchronizable type!");
				static_assert(std::is_base_of<IType, LocalSyncType>::value, "Cannot register a SyncType that is not derived from IType!");
				if (initialized)
				{
					spehs::log::error("codex::sync::Manager cannot register new types once initialized!");
					return;
				}

				//Register type data
				registeredTypes.push_back(TypeInfo());

				//Local type
				registeredTypes.back().local.name = LocalSyncType::getSyncTypeName();
				registeredTypes.back().local.typeId = LocalSyncType::getSyncTypeId();
				registeredTypes.back().local.version = LocalSyncType::getSyncTypeVersion();
				registeredTypes.back().local.constructor = []() {return new LocalSyncType(); };

				//Remote type
				registeredTypes.back().remote.name = remoteTypeName;
				registeredTypes.back().remote.typeId = remoteTypeId;
				registeredTypes.back().remote.version = remoteTypeVersion;
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
				TypeInfo* typeInfo = findByLocalType(LocalSyncType::getSyncTypeId());
				if (!typeInfo)
				{
					spehs::log::info("codex::sync::Manager::create: Cannot create a type that is not registered.");
					return Handle<LocalSyncType>();
				}
				Entry* entry = new Entry(*this, new LocalSyncType(constructorParams...), nextEntryId++, *typeInfo, true);
				entries.push_back(entry);
				return Handle<LocalSyncType>(*entry);
			}

			/* Fires when a local type is instantiated by the remote sync manager. */
			//boost::signal<void(Handle<TODO>&)> localTypeInstantiatedSignal;

		private:
			bool receiveHandler(protocol::ReadBuffer& buffer);
			
			Entry* findEntry(const Entry::Id id);
			TypeInfo* findByLocalType(const IType::SyncTypeIdType& typeId);
			TypeInfo* findByRemoteType(const IType::SyncTypeIdType& typeId);
			
			bool initialized;
			SocketTCP& socket;
			Entry::Id nextEntryId;
			std::vector<TypeInfo> registeredTypes;
			std::vector<Entry*> entries;

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