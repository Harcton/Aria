#pragma once
#include <functional>
#include <atomic>
#include "SyncType.h"
#include "CodexTime.h"
#include "IOService.h"
#include "SocketTCP.h"
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
			uncompatible,
		};
		struct Entry
		{
			typedef uint16_t Id;
			Entry(GhostSyncType& ref) : ptr(&ref) {}

			GhostSyncType* ptr;
			Id id = 0;
			time::TimeType interval;
			time::TimeType timer;
		};

	public:

		SyncManager();
		~SyncManager();

		template<typename GhostType, typename ShellType>
		void registerType()
		{
			static_assert(!std::is_same<GhostSyncType, GhostType>::value, "Cannot register GhostSyncType as a synchronizable type!");
			static_assert(std::is_base_of<GhostSyncType, GhostType>::value, "Cannot register a ghost type that is not derived from GhostSyncType!");
			static_assert(!std::is_same<ShellSyncType, ShellType>::value, "Cannot register ShellSyncType as a synchronizable type!");
			static_assert(std::is_base_of<ShellSyncType, ShellType>::value, "Cannot register a shell type that is not derived from ShellSyncType!");
			if (initialized)
			{
				codex::log::error("SyncManager cannot register new types once initialized!");
				return;
			}

			//TODO: register type data
			registeredTypes.push_back(SyncTypeInfo());
			registeredTypes.back().ghostName = GhostType::getGhostSyncTypeName();
			registeredTypes.back().shellName = ShellType::getShellSyncTypeName();
			registeredTypes.back().ghostVersion = GhostType::getGhostSyncTypeVersion();
			registeredTypes.back().shellVersion = ShellType::getShellSyncTypeVersion();
			registeredTypes.back().shellConstructor = []() {return new ShellType(); };
		}

		//Connect and initialize
		bool startAccepting(const protocol::PortType port);
		bool connect(const protocol::Endpoint& endpoint);
		bool initialize();
		bool isConnected() const;
		bool isAccepting() const;
		bool isInitialized() const;

		void update(const time::TimeType& deltaTime);
		void addEntry(GhostSyncType& reference, const time::TimeType& syncInterval);
		void removeEntry(GhostSyncType& reference);
		void setSyncInterval(GhostSyncType& reference, const time::TimeType& syncInterval);
		
	private:
		bool receiveHandler(protocol::ReadBuffer& buffer);
		void onAcceptCallback(SocketTCP& socket);

		/*
		create, allocate
		remove
		*/

		//SyncTypeInformation
		struct SyncTypeInfo
		{
			void write(protocol::WriteBuffer& buffer)
			{
				buffer.write(ghostName);
				buffer.write(shellName);
				buffer.write(ghostVersion);
				buffer.write(shellVersion);
			}
			void read(protocol::ReadBuffer& buffer)
			{
				buffer.read(ghostName);
				buffer.read(shellName);
				buffer.read(ghostVersion);
				buffer.read(shellVersion);
			}
			std::string ghostName;
			std::string shellName;
			GhostSyncType::GhostSyncTypeVersionType ghostVersion;
			ShellSyncType::ShellSyncTypeVersionType shellVersion;
			std::function<ShellSyncType*()> shellConstructor;
		};
		std::vector<SyncTypeInfo> registeredTypes;
		
		bool initialized;
		IOService ioService;
		SocketTCP socket;
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
				for (size_t i = 0; i < shells.size(); i++)
					delete shells[i];
			}
			std::atomic<TypeCompatibility> typeCompatibility;//My evaluation
			std::atomic<TypeCompatibility> typeCompatibilityResponse;//Remote evaluation
			std::vector<ShellSyncType*> shells;
		} remoteSyncManager;
	};
}