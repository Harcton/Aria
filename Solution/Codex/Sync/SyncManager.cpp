#include "SyncManager.h"
#include "CodexAssert.h"
#include "IOService.h"
#include "SocketTCP.h"
#include <Codex/StringOperations.h>



namespace codex
{
	namespace sync
	{
		static int debugLevel = 1;
		
		Manager::Manager(SocketTCP& _socket)
			: initialized(false)
			, socket(_socket)
			, nextLocalEntryId(1)
		{

		}

		Manager::~Manager()
		{
			for (size_t i = 0; i < localEntries.size(); i++)
				delete localEntries[i];
			for (size_t i = 0; i < remoteEntries.size(); i++)
				delete remoteEntries[i];
		}

		static const uint64_t syncManagerInitializeTypesMagicNumber = 0x13372112BEEF0001;
		static const uint64_t syncManagerInitializeTypesResponseMagicNumber = 0x13372112BEEF0002;

		bool Manager::initialize()
		{
			if (initialized)
			{
				codex::log::error("codex::sync::Manager is already initialized.");
				return false;
			}

			if (!socket.isConnected())
			{
				codex::log::error("codex::sync::Manager cannot initialize, the provided socket is not in the connected state.");
				return false;
			}

			if (socket.isReceiving())
			{
				codex::log::error("codex::sync::Manager cannot initialize, the provided socket is already in the receiving state.");
				return false;
			}

			//Clear all instances
			for (size_t i = 0; i < localEntries.size(); i++)
				delete localEntries[i];
			for (size_t i = 0; i < remoteEntries.size(); i++)
				delete remoteEntries[i];
			localEntries.clear();
			remoteEntries.clear();

			//Reset the remote sync manager state
			remoteManager.typeCompatibility = TypeCompatibility::unknown;
			remoteManager.typeCompatibilityResponse = TypeCompatibility::unknown;

			//Print my registered type info(?)
			if (debugLevel >= 1)
			{
				log::info("codex::sync::Manager registered types:");
				for (size_t i = 0; i < registeredTypes.size(); i++)
				{
					log::info("[" + std::to_string(i) + "]\tLocal: name: " + registeredTypes[i].local.name + " typeid: " + registeredTypes[i].local.typeId);
					log::info("\tRemote: name: " + registeredTypes[i].remote.name + " typeid: " + registeredTypes[i].remote.typeId);
				}
			}

			//Prepare a packet for the remote sync manager
			protocol::WriteBuffer buffer;
			if (debugLevel >= 1)
				log::info("codex::sync::Manager: writing an init packet for the remote counterpart...");
			//Write a header value for some packet verification
			buffer.write<PacketType>(PacketType::types);
			buffer.write(syncManagerInitializeTypesMagicNumber);
			//Write my registered types
			const size_t registeredTypesCount = registeredTypes.size();
			buffer.write(registeredTypesCount);
			for (size_t i = 0; i < registeredTypesCount; i++)
				registeredTypes[i].write(buffer);
			//Send
			socket.sendPacket(buffer);
			if (debugLevel >= 1)
				log::info("codex::sync::Manager: sending an init packet for the remote counterpart...");
			//Start receiving
			socket.startReceiving(std::bind(&Manager::receiveHandler, this, std::placeholders::_1));

			//Wait...
			const time::TimeType begin = time::now();
			const time::TimeType timeout = time::seconds(999999.0f);// 10.0f);
			while (remoteManager.typeCompatibility == TypeCompatibility::unknown || remoteManager.typeCompatibilityResponse == TypeCompatibility::unknown)
			{
				socket.update();
				if (time::now() - begin >= timeout)
				{//Timeouted
					if (debugLevel >= 1)
						log::info("codex::sync::Manager: initialize timeouted. The remote sync manager did not respond within the given time limit");
					socket.stopReceiving();
					return false;
				}
			}

			//Inspect results
			if (remoteManager.typeCompatibility == TypeCompatibility::compatible)
				log::info("codex::sync::Manager: compatible with the remote sync manager's types.");
			else
				log::info("codex::sync::Manager: incompatible with the remote sync manager's types.");
			if (remoteManager.typeCompatibilityResponse == TypeCompatibility::compatible)
				log::info("codex::sync::Manager: remote sync manager is compatible with my types.");
			else
				log::info("codex::sync::Manager: remote sync manager is incompatible with my types.");
			if (remoteManager.typeCompatibility == TypeCompatibility::compatible && remoteManager.typeCompatibilityResponse == TypeCompatibility::compatible)
			{
				log::info("codex::sync::Manager: initialization complete.");
				initialized = true;
				return true;
			}
			else
			{
				return false;
			}
		}

		bool Manager::receiveHandler(protocol::ReadBuffer& buffer)
		{
			while (buffer.getBytesRemaining())
			{
				PacketType packetType;
				buffer.read<PacketType>(packetType);
				if (!isInitialized() && packetType != PacketType::types)
				{
					log::warning("codex::sync::Manager: first received packet type is expected to contain type data! Packet type was set to: " + std::to_string((int)packetType));
					return true;
				}

				switch (packetType)
				{
				default:
					codex::log::warning("codex::sync::Manager: received unknown packet type: " + std::to_string((int)packetType));
					return true;
				case PacketType::types:
				{//Not initialized
					uint64_t magic;
					buffer.read(magic);
					if (magic == syncManagerInitializeTypesMagicNumber)
					{//Received remote types
						size_t remoteRegisteredTypesCount;
						buffer.read(remoteRegisteredTypesCount);
						std::vector<SyncTypePairInfo> remoteRegisteredTypes(remoteRegisteredTypesCount);
						remoteManager.typeCompatibility = TypeCompatibility::compatible;
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
										if (registeredTypes[m].local.typeId != remoteTypeInfo.local.typeId)
											failureString += "\nLocal type '" + registeredTypes[m].local.name + "' typeid mismatch, my local typeid '" + registeredTypes[m].local.typeId + ", other sync manager's local typeId '" + remoteTypeInfo.local.typeId;
										if (registeredTypes[m].remote.typeId != remoteTypeInfo.remote.typeId)
											failureString += "\nRemote type '" + registeredTypes[m].remote.name + "' typeid mismatch, my remote typeid '" + registeredTypes[m].remote.typeId + ", other sync manager's remote typeId '" + remoteTypeInfo.remote.typeId;
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
							remoteManager.typeCompatibility = TypeCompatibility::incompatible;
							codex::log::info(failureString);
						}

						//Write a response packet
						protocol::WriteBuffer response;
						response.write<PacketType>(PacketType::types);
						response.write(syncManagerInitializeTypesResponseMagicNumber);
						response.write((uint8_t)remoteManager.typeCompatibility.load());
						socket.sendPacket(response);
					}
					else if (magic == syncManagerInitializeTypesResponseMagicNumber)
					{//Received remote type inspectation results
						buffer.read((uint8_t&)remoteManager.typeCompatibilityResponse);
						if (remoteManager.typeCompatibilityResponse == TypeCompatibility::incompatible)
						{
							codex::log::info("codex::sync::Manager: initialization failed. (my)Local types are not compatible with the remote types.");
						}
					}
					else
					{
						if (debugLevel >= 1)
							log::info("codex::sync::Manager: received an invalid packet while waiting for the init packet.");
					}
				}
				break;
				case PacketType::localCreate:
				{//Local type created (remotely)

					//Entry data
					IType::SyncTypeIdType typeId;
					buffer.read(typeId);
					AbstractEntry::Id id;
					buffer.read(id);
					LocalSyncTypeInfo* localSyncTypeInfo = findLocalTypeInfo(typeId);
					if (!localSyncTypeInfo)
					{
						log::warning("codex::sync::Manager: PacketType::localCreate had invalid typeId value. The typeId value did not correspond to any known typeId. typeId: " + typeId);
						return false;
					}
					RemoteSyncTypeInfo* remoteSyncTypeInfo = findRemoteTypeInfo(*localSyncTypeInfo);
					CODEX_ASSERT(remoteSyncTypeInfo);
					IType* instance = remoteSyncTypeInfo->constructor();
					RemoteEntry* remoteEntry = new RemoteEntry(*this, instance, id, *remoteSyncTypeInfo);
					instance->syncCreate(buffer);
					remoteEntries.push_back(remoteEntry);
					//remoteTypeCreatedSignal(*instance);
				}
				break;
				case PacketType::remoteCreate:
				{//Remote type created (remotely)
					AbstractEntry::Id id;
					buffer.read(id);
					LocalEntry* localEntry = findLocalEntry(id);
					if (localEntry)
					{
						localEntry->ptr->syncCreate(buffer);
						localEntry->createReceived = true;
					}
					else
					{
						log::error("codex::sync::Manager: receive handler: PacketType::remoteCreate: Unknown local entry id!");
						return true;
					}
				}
				break;
				case PacketType::localUpdate:
				{//(remote)Local type sends update
					AbstractEntry::Id id;
					buffer.read(id);
					RemoteEntry* remoteEntry = findRemoteEntry(id);
					if (remoteEntry)
					{
						remoteEntry->ptr->syncUpdate(buffer);
					}
					else
					{
						if (debugLevel >= 3)
							log::info("codex::sync::Manager: receive handler: PacketType::localUpdate: Unknown remote entry id! Maybe entry was just removed...");
						return true;
					}
				}
				break;
				case PacketType::remoteUpdate:
				{//(remote)Remote type sends update
					AbstractEntry::Id id;
					buffer.read(id);
					LocalEntry* localEntry = findLocalEntry(id);
					if (localEntry)
					{
						localEntry->ptr->syncUpdate(buffer);
					}
					else
					{
						log::error("codex::sync::Manager: receive handler: PacketType::remoteUpdate: Unknown remote entry id!");
						return true;
					}
				}
				break;
				case PacketType::localRemove:
				{//(remote)Local type (to be) removed
					AbstractEntry::Id id;
					buffer.read(id);
					RemoteEntry* remoteEntry = findRemoteEntry(id);
					if (remoteEntry)
					{
						remoteEntry->ptr->syncRemove(buffer);
						remoteEntry->removeReceived = true;
					}
					else
					{
						log::error("codex::sync::Manager: receive handler: PacketType::localRemove: Unknown remote entry id!");
						return true;
					}
				}
				break;
				case PacketType::remoteRemove:
				{//(remote)Remote type removed -> removes local type
					AbstractEntry::Id id;
					buffer.read(id);
					LocalEntry* localEntry = findLocalEntry(id);
					if (localEntry)
					{
						localEntry->ptr->syncRemove(buffer);
						localEntry->removeReceived = true;
					}
					else
					{
						log::error("codex::sync::Manager: receive handler: PacketType::remoteRemove: Unknown local entry id!");
						return true;
					}
				}
				break;
				}
			}
			return true;
		}

		bool Manager::isInitialized() const
		{
			return initialized;
		}

		void Manager::update(const time::TimeType& deltaTime)
		{
			CODEX_ASSERT(isInitialized());

			//Local types
			for (size_t i = 0; i < localEntries.size();)
			{
				CODEX_ASSERT(localEntries[i] && localEntries[i]->ptr);

				//Create
				if (!localEntries[i]->createSent)
				{
					protocol::WriteBuffer buffer;
					buffer.write<PacketType>(PacketType::localCreate);
					buffer.write(localEntries[i]->typeInfo.typeId);
					buffer.write(localEntries[i]->id);
					localEntries[i]->ptr->syncCreate(buffer);
					localEntries[i]->createSent = socket.sendPacket(buffer);
					if (debugLevel >= 1)
					{
						const LocalSyncTypeInfo* typeInfo = findLocalTypeInfo(localEntries[i]->typeInfo.typeId);
						const std::string typeName = typeInfo ? typeInfo->name : "unknown";
						if (localEntries[i]->createSent)
							codex::log::info("codex::sync::Manager: PacketType::localCreate sent for instance of type: " + typeName);
						else
							codex::log::info("codex::sync::Manager: failed to send PacketType::localCreate for instance of type: " + typeName);
					}
				}

				//Update
				localEntries[i]->timer += deltaTime;
				if (localEntries[i]->ptr->syncUpdate(deltaTime) || (localEntries[i]->interval != 0 && localEntries[i]->timer >= localEntries[i]->interval))
				{
					localEntries[i]->timer = 0;
					protocol::WriteBuffer buffer;
					buffer.write<PacketType>(PacketType::localUpdate);
					buffer.write(localEntries[i]->id);
					localEntries[i]->ptr->syncUpdate(buffer);
					const bool updateSent = socket.sendPacket(buffer);
					if (debugLevel >= 3)
					{
						const LocalSyncTypeInfo* typeInfo = findLocalTypeInfo(localEntries[i]->typeInfo.typeId);
						const std::string typeName = typeInfo ? typeInfo->name : "unknown";
						if (updateSent)
							codex::log::info("codex::sync::Manager: PacketType::localUpdate sent for instance of type: " + typeName);
						else
							codex::log::info("codex::sync::Manager: failed to send PacketType::localUpdate for instance of type: " + typeName);
					}
				}

				//Remove
				if (localEntries[i]->handles.empty() && !localEntries[i]->removeSent)
				{
					protocol::WriteBuffer buffer;
					buffer.write<PacketType>(PacketType::localRemove);
					buffer.write(localEntries[i]->id);
					localEntries[i]->ptr->syncRemove(buffer);
					localEntries[i]->removeSent = socket.sendPacket(buffer);
					if (debugLevel >= 1)
					{
						const LocalSyncTypeInfo* typeInfo = findLocalTypeInfo(localEntries[i]->typeInfo.typeId);
						const std::string typeName = typeInfo ? typeInfo->name : "unknown";
						if (localEntries[i]->removeSent)
							codex::log::info("codex::sync::Manager: PacketType::localRemove sent for instance of type: " + typeName);
						else
							codex::log::info("codex::sync::Manager: failed to send PacketType::localRemove for instance of type: " + typeName);
					}
				}

				//Deallocate
				if (localEntries[i]->removeReceived)
				{
					delete localEntries[i];
					localEntries[i] = localEntries.back();
					localEntries.pop_back();
				}
				else
					i++;
			}

			//Remote entries
			for (size_t i = 0; i < remoteEntries.size();)
			{
				//Create
				if (!remoteEntries[i]->createSent)
				{
					protocol::WriteBuffer buffer;
					buffer.write<PacketType>(PacketType::remoteCreate);
					buffer.write(remoteEntries[i]->id);
					remoteEntries[i]->ptr->syncCreate(buffer);
					remoteEntries[i]->createSent = socket.sendPacket(buffer);
					if (debugLevel >= 1)
					{
						const RemoteSyncTypeInfo* typeInfo = findRemoteTypeInfo(remoteEntries[i]->typeInfo.typeId);
						const std::string typeName = typeInfo ? typeInfo->name : "unknown";
						if (remoteEntries[i]->createSent)
							codex::log::info("codex::sync::Manager: PacketType::remoteCreate sent for instance of type: " + typeName);
						else
							codex::log::info("codex::sync::Manager: failed to send PacketType::remoteCreate for instance of type: " + typeName);
					}
				}

				//Update
				if (remoteEntries[i]->ptr->syncUpdate(deltaTime))
				{
					protocol::WriteBuffer buffer;
					buffer.write<PacketType>(PacketType::remoteUpdate);
					buffer.write(remoteEntries[i]->id);
					remoteEntries[i]->ptr->syncUpdate(buffer);
					const bool updateSent = socket.sendPacket(buffer);
					if (debugLevel >= 3)
					{
						const RemoteSyncTypeInfo* typeInfo = findRemoteTypeInfo(remoteEntries[i]->typeInfo.typeId);
						const std::string typeName = typeInfo ? typeInfo->name : "unknown";
						if (updateSent)
							codex::log::info("codex::sync::Manager: PacketType::remoteUpdate sent for instance of type: " + typeName);
						else
							codex::log::info("codex::sync::Manager: failed to send PacketType::remoteUpdate for instance of type: " + typeName);
					}
				}

				//Remove
				if (remoteEntries[i]->removeReceived && !remoteEntries[i]->removeSent)
				{
					protocol::WriteBuffer buffer;
					buffer.write<PacketType>(PacketType::remoteRemove);
					buffer.write(remoteEntries[i]->id);
					remoteEntries[i]->ptr->syncRemove(buffer);
					remoteEntries[i]->removeSent = socket.sendPacket(buffer);
					if (debugLevel >= 1)
					{
						const RemoteSyncTypeInfo* typeInfo = findRemoteTypeInfo(remoteEntries[i]->typeInfo.typeId);
						const std::string typeName = typeInfo ? typeInfo->name : "unknown";
						if (remoteEntries[i]->removeSent)
							codex::log::info("codex::sync::Manager: PacketType::remoteRemove sent for instance of type: " + typeName);
						else
							codex::log::info("codex::sync::Manager: failed to send PacketType::remoteRemove for instance of type: " + typeName);
					}
				}

				//Deallocate
				if (remoteEntries[i]->removeSent)
				{
					if (debugLevel >= 1)
						codex::log::info("codex::sync::Manager: removing entry id: " + std::to_string(remoteEntries[i]->id));
					delete remoteEntries[i];
					remoteEntries[i] = remoteEntries.back();
					remoteEntries.pop_back();
				}
				else
					i++;
			}
		}

		LocalSyncTypeInfo* Manager::findLocalTypeInfo(const IType::SyncTypeIdType& typeId)
		{
			for (size_t i = 0; i < registeredTypes.size(); i++)
			{
				if (registeredTypes[i].local.typeId == typeId)
					return &registeredTypes[i].local;
			}
			return nullptr;
		}

		LocalSyncTypeInfo* Manager::findLocalTypeInfo(const RemoteSyncTypeInfo& remoteCounterpart)
		{
			for (size_t i = 0; i < registeredTypes.size(); i++)
			{
				if (registeredTypes[i].remote.typeId == remoteCounterpart.typeId)
					return &registeredTypes[i].local;
			}
			return nullptr;
		}

		RemoteSyncTypeInfo* Manager::findRemoteTypeInfo(const IType::SyncTypeIdType& typeId)
		{
			for (size_t i = 0; i < registeredTypes.size(); i++)
			{
				if (registeredTypes[i].remote.typeId == typeId)
					return &registeredTypes[i].remote;
			}
			return nullptr;
		}

		RemoteSyncTypeInfo* Manager::findRemoteTypeInfo(const LocalSyncTypeInfo& localCounterpart)
		{
			for (size_t i = 0; i < registeredTypes.size(); i++)
			{
				if (registeredTypes[i].local.typeId == localCounterpart.typeId)
					return &registeredTypes[i].remote;
			}
			return nullptr;
		}

		LocalEntry* Manager::findLocalEntry(const AbstractEntry::Id id)
		{
			for (size_t i = 0; i < localEntries.size(); i++)
			{
				if (localEntries[i]->id == id)
					return localEntries[i];
			}
			return nullptr;
		}

		RemoteEntry* Manager::findRemoteEntry(const AbstractEntry::Id id)
		{
			for (size_t i = 0; i < remoteEntries.size(); i++)
			{
				if (remoteEntries[i]->id == id)
					return remoteEntries[i];
			}
			return nullptr;
		}
	}
}