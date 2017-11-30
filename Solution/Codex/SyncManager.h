#pragma once
#include <functional>
#include <vector>
#include <atomic>
#include "SyncType.h"
#include "CodexTime.h"
#include "Log.h"

namespace codex
{
	class SocketTCP;
	namespace protocol
	{
		class WriteBuffer;
		class ReadBuffer;
	}

	class SyncManager
	{
	public:
		enum class TypeCompatibility : uint8_t
		{
			unknown,
			compatible,
			incompatible,
		};
		struct Entry
		{
			typedef uint16_t Id;
			Entry(ISyncType& ref) : ptr(&ref) {}

			ISyncType* ptr;
			Id id = 0;
			time::TimeType interval;
			time::TimeType timer;
		};

	public:

		SyncManager(SocketTCP& socket);
		~SyncManager();

		template<typename LocalSyncType, typename RemoteSyncType>
		void registerType()
		{
			static_assert(!std::is_same<ISyncType, LocalSyncType>::value, "Cannot register SyncType as a synchronizable type!");
			static_assert(std::is_base_of<ISyncType, LocalSyncType>::value, "Cannot register a SyncType that is not derived from ISyncType!");
			static_assert(!std::is_same<ISyncType, RemoteSyncType>::value, "Cannot register SyncType as a synchronizable type!");
			static_assert(std::is_base_of<ISyncType, RemoteSyncType>::value, "Cannot register a SyncType that is not derived from ISyncType!");
			if (initialized)
			{
				codex::log::error("SyncManager cannot register new types once initialized!");
				return;
			}

			//Register type data
			registeredTypes.push_back(SyncTypePairInfo());
			registeredTypes.back().local.name = LocalSyncType::getSyncTypeName();
			registeredTypes.back().remote.name = RemoteSyncType::getSyncTypeName();
			registeredTypes.back().local.version = LocalSyncType::getSyncTypeVersion();
			registeredTypes.back().remote.version = RemoteSyncType::getSyncTypeVersion();
			registeredTypes.back().remote.constructor = []() {return new RemoteSyncType(); };
		}

		//Connect and initialize
		bool initialize();
		bool isInitialized() const;

		void update(const time::TimeType& deltaTime);
		void addEntry(ISyncType& reference, const time::TimeType& syncInterval);
		void removeEntry(ISyncType& reference);
		void setSyncInterval(ISyncType& reference, const time::TimeType& syncInterval);
		
	private:
		bool receiveHandler(protocol::ReadBuffer& buffer);

		/*
		create, allocate
		remove
		*/

		//SyncTypeInformation
		struct SyncTypePairInfo
		{
			void write(protocol::WriteBuffer& buffer);
			void read(protocol::ReadBuffer& buffer);
			struct LocalSyncTypeInfo
			{
				std::string name;
				ISyncType::SyncTypeVersionType version;
			};
			struct RemoteSyncTypeInfo : public LocalSyncTypeInfo
			{
				std::function<ISyncType*()> constructor;
			};
			LocalSyncTypeInfo local;
			RemoteSyncTypeInfo remote;
		};
		std::vector<SyncTypePairInfo> registeredTypes;
		
		bool initialized;
		SocketTCP& socket;
		Entry::Id nextGeneratedId;
		std::vector<Entry> entriesToAdd;
		std::vector<Entry> entries;

		/* Sync managers are connected to each other. */
		class RemoteSyncManager
		{
		public:
			RemoteSyncManager()
				: typeCompatibility(TypeCompatibility::unknown)
				, typeCompatibilityResponse(TypeCompatibility::unknown)
			{

			}
			RemoteSyncManager(const RemoteSyncManager& other) = delete;
			void operator=(const RemoteSyncManager& other) = delete;
			~RemoteSyncManager()
			{
				for (size_t i = 0; i < instances.size(); i++)
					delete instances[i];
			}
			std::atomic<TypeCompatibility> typeCompatibility;//My evaluation
			std::atomic<TypeCompatibility> typeCompatibilityResponse;//Remote evaluation
			std::vector<ISyncType*> instances;
		} remoteSyncManager;
	};
}