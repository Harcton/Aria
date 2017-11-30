#include "SyncManager.h"
#include "CodexAssert.h"
#include "IOService.h"
#include "SocketTCP.h"



namespace codex
{
	static int debugLevel = 0;

	void SyncManager::SyncTypePairInfo::write(protocol::WriteBuffer& buffer)
	{
		buffer.write(local.name);
		buffer.write(local.version);
		buffer.write(remote.name);
		buffer.write(remote.version);
	}

	void SyncManager::SyncTypePairInfo::read(protocol::ReadBuffer& buffer)
	{
		buffer.read(local.name);
		buffer.read(local.version);
		buffer.read(remote.name);
		buffer.read(remote.version);
	}



	SyncManager::SyncManager(SocketTCP& _socket)
		: initialized(false)
		, socket(_socket)
		, nextGeneratedId(1)
	{

	}

	SyncManager::~SyncManager()
	{
		//Sync manager is going out of scope -> severe links to entried instances
		//for (size_t i = 0; i < entriesToAdd.size(); i++)
		//{
		//	CODEX_ASSERT(entriesToAdd[i].ptr->syncManager == this);
		//	entriesToAdd[i].ptr->syncManager = nullptr;
		//}
		//for (size_t i = 0; i < entries.size(); i++)
		//{
		//	CODEX_ASSERT(entries[i].ptr->syncManager == this);
		//	entries[i].ptr->syncManager = nullptr;
		//}
	}

	static const uint64_t syncManagerInitializeTypesMagicNumber = 0x13372112BEEF0001;
	static const uint64_t syncManagerInitializeTypesResponseMagicNumber = 0x13372112BEEF0002;
	
	bool SyncManager::initialize()
	{
		if (initialized)
		{
			codex::log::error("SyncManager is already initialized.");
			return false;
		}

		if (!socket.isConnected())
		{
			codex::log::error("SyncManager cannot initialize, the provided socket is not in the connected state.");
			return false;
		}

		if (socket.isReceiving())
		{
			codex::log::error("SyncManager cannot initialize, the provided socket is already in the receiving state.");
			return false;
		}

		//Reset the remote sync manager state
		for (size_t i = 0; i < remoteSyncManager.instances.size(); i++)
			delete remoteSyncManager.instances[i];
		remoteSyncManager.instances.clear();
		remoteSyncManager.typeCompatibility = TypeCompatibility::unknown;
		remoteSyncManager.typeCompatibilityResponse = TypeCompatibility::unknown;
		
		//Prepare a packet for the remote sync manager
		protocol::WriteBuffer buffer;
		if (debugLevel >= 1)
			log::info("SyncManager: writing an init packet for the remote counterpart...");
		//Write a header value for some packet verification
		buffer.write(syncManagerInitializeTypesMagicNumber);
		//Write my registered types
		const size_t registeredTypesCount = registeredTypes.size();
		buffer.write(registeredTypesCount);
		for (size_t i = 0; i < registeredTypesCount; i++)
			registeredTypes[i].write(buffer);
		//Send
		socket.sendPacket(buffer);
		if (debugLevel >= 1)
			log::info("SyncManager: sending an init packet for the remote counterpart...");
		//Start receiving
		socket.startReceiving(std::bind(&SyncManager::receiveHandler, this, std::placeholders::_1));

		//Wait...
		const time::TimeType begin = time::now();
		const time::TimeType timeout = time::seconds(999999.0f);// 10.0f);
		while (remoteSyncManager.typeCompatibility == TypeCompatibility::unknown || remoteSyncManager.typeCompatibilityResponse == TypeCompatibility::unknown)
		{
			socket.update();
			if (time::now() - begin >= timeout)
			{//Timeouted
				if (debugLevel >= 1)
					log::info("SyncManager: initialize timeouted. The remote sync manager did not respond within the given time limit");
				socket.stopReceiving();
				return false;
			}
		}

		//Inspect results
		if (remoteSyncManager.typeCompatibility == TypeCompatibility::compatible)
			log::info("SyncManager: compatible with the remote sync manager's types.");
		else
			log::info("SyncManager: incompatible with the remote sync manager's types.");
		if (remoteSyncManager.typeCompatibilityResponse == TypeCompatibility::compatible)
			log::info("SyncManager: remote sync manager is compatible with my types.");
		else
			log::info("SyncManager: remote sync manager is incompatible with my types.");
		if (remoteSyncManager.typeCompatibility == TypeCompatibility::compatible && remoteSyncManager.typeCompatibilityResponse == TypeCompatibility::compatible)
		{
			log::info("SyncManager: initialization complete.");
			return true;
		}
		else
		{
			return false;
		}
	}

	bool SyncManager::receiveHandler(protocol::ReadBuffer& buffer)
	{
		if (isInitialized())
		{//Initialized

		}
		else
		{//Not initialized
			uint64_t magic;
			buffer.read(magic);
			if (magic == syncManagerInitializeTypesMagicNumber)
			{//Received remote types
				size_t remoteRegisteredTypesCount;
				buffer.read(remoteRegisteredTypesCount);
				std::vector<SyncTypePairInfo> remoteRegisteredTypes(remoteRegisteredTypesCount);
				remoteSyncManager.typeCompatibility = TypeCompatibility::compatible;
				std::string failureString;
				for (size_t r = 0; r < remoteRegisteredTypesCount; r++)
				{
					remoteRegisteredTypes[r].read(buffer);
					const SyncTypePairInfo& remoteTypeInfo = remoteRegisteredTypes[r];
					bool found = false;
					for (size_t m = 0; m < registeredTypes.size(); m++)
					{
						const bool localMatch = registeredTypes[m].local.name == remoteTypeInfo.local.name;
						const bool remoteMatch = registeredTypes[m].remote.name == remoteTypeInfo.remote.name;
						if (localMatch || remoteMatch)
						{
							if (localMatch && remoteMatch)
							{
								found = true;
								if (registeredTypes[m].local.version != remoteTypeInfo.local.version)
									failureString += "\nLocal type '" + registeredTypes[m].local.name + "' version mismatch, my local version '" + std::to_string(registeredTypes[m].local.version) + "', other sync manager's local version '" + std::to_string(remoteTypeInfo.local.version) + "'.";
								if (registeredTypes[m].remote.version != remoteTypeInfo.remote.version)
									failureString += "\nRemote type '" + registeredTypes[m].remote.name + "' version mismatch, my remote version '" + std::to_string(registeredTypes[m].remote.version) + "', other sync manager's remote version '" + std::to_string(remoteTypeInfo.remote.version) + "'.";
							}
							else if (!localMatch)
								failureString += "\nRemote type names match: '" + remoteTypeInfo.remote.name + "', but local names mismatch: my '" + registeredTypes[m].local.name + "' remote: '" + remoteTypeInfo.local.name + "'!";
							else if (!remoteMatch)
								failureString += "\nLocal type names match: '" + remoteTypeInfo.local.name + "', but remote names mismatch: my '" + registeredTypes[m].remote.name + "' remote: '" + remoteTypeInfo.remote.name + "'!";
							break;
						}
					}
					if (!found)
						failureString += "\nUnknown type: local '" + remoteTypeInfo.local.name + "', remote '" + remoteTypeInfo.remote.name + "'!";
				}
				if (failureString.size() > 1)
				{
					remoteSyncManager.typeCompatibility = TypeCompatibility::incompatible;
					codex::log::info(failureString);
				}

				//Write a response packet
				protocol::WriteBuffer response;
				response.write(syncManagerInitializeTypesResponseMagicNumber);
				response.write((uint8_t)remoteSyncManager.typeCompatibility.load());
				socket.sendPacket(response);
			}
			else if (magic == syncManagerInitializeTypesResponseMagicNumber)
			{//Received remote type inspectation results
				buffer.read((uint8_t&)remoteSyncManager.typeCompatibilityResponse);
				if (remoteSyncManager.typeCompatibilityResponse == TypeCompatibility::incompatible)
				{
					codex::log::info("SyncManager: initialization failed. (my)Local types are not compatible with the remote types.");
				}
				return true;
			}
			else
			{
				if (debugLevel >= 1)
					log::info("SyncManager: received an invalid packet while waiting for the init packet.");
				return true;
			}
		}
	}

	bool SyncManager::isInitialized() const
	{
		return initialized;
	}

	void SyncManager::update(const time::TimeType& deltaTime)
	{
		socket.update();

		if (entriesToAdd.size() > 0)
		{
			protocol::WriteBuffer buffer;
			for (size_t i = 0; i < entriesToAdd.size(); i++)
			{
				//TODO: Sync creation
				entries.push_back(entriesToAdd[i]);
			}
			entriesToAdd.clear();
		}

		for (size_t i = 0; i < entries.size(); i++)
		{
			entries[i].timer += deltaTime;
			if (entries[i].timer >= entries[i].interval)
			{
				entries[i].timer -= entries[i].interval;
				//Send sync data
			}
		}
	}

	void SyncManager::addEntry(ISyncType& reference, const time::TimeType& syncInterval)
	{
		//Check that entry is not in the to be added queue
		for (size_t i = 0; i < entriesToAdd.size(); i++)
		{
			if (entriesToAdd[i].ptr == &reference)
			{
				log::error("Entry already added!");
				return;
			}
		}
		//Check that entry is not in the added entries
		for (size_t i = 0; i < entries.size(); i++)
		{
			if (entries[i].ptr == &reference)
			{
				log::error("Entry already added!");
				return;
			}
		}

		entriesToAdd.push_back(Entry(reference));
		entriesToAdd.back().interval = syncInterval;
		entriesToAdd.back().timer = syncInterval;
		entriesToAdd.back().id = nextGeneratedId++;
	}

	void SyncManager::removeEntry(ISyncType& reference)
	{
		//Search entries to add
		for (size_t i = 0; i < entriesToAdd.size(); i++)
		{
			if (entriesToAdd[i].ptr == &reference)
			{
				//TODO: sync removal
				entriesToAdd.erase(entriesToAdd.begin() + i);
				return;
			}
		}
		//Search entries
		for (size_t i = 0; i < entries.size(); i++)
		{
			if (entries[i].ptr == &reference)
			{
				//TODO: sync removal
				entries.erase(entries.begin() + i);
				return;
			}
		}
		log::warning("Specified entry not found from among the registered entries!");
	}

	void SyncManager::setSyncInterval(ISyncType& reference, const time::TimeType& syncInterval)
	{
		for (size_t i = 0; i < entriesToAdd.size(); i++)
		{
			if (entriesToAdd[i].ptr == &reference)
			{
				entriesToAdd[i].interval = syncInterval;
				if (entriesToAdd[i].timer > syncInterval)
					entriesToAdd[i].timer = syncInterval;
				return;
			}
		}
		for (size_t i = 0; i < entries.size(); i++)
		{
			if (entries[i].ptr == &reference)
			{
				entries[i].interval = syncInterval;
				if (entries[i].timer > syncInterval)
					entries[i].timer = syncInterval;
				return;
			}
		}
		log::warning("Specified entry not found from among the registered entries!");
	}
}