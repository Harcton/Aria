#include "Aria.h"
#include "RAIIVariableSetter.h"



namespace codex
{
	namespace aria
	{
		static const uint64_t ariaMagicHeader = 0xC0DEA21A1994A51A;
		enum class PacketType : uint8_t
		{
			enter,							//Sent by client upon connecting. Contains name, counterpart and local port if need to wait.
			counterpartEndpointAccept,		//Sent to clients who have been paired with a counterpart. Expected action is to start accepting on the declared port.
			counterpartEndpointConnect,		//Sent to clients who have been paired with a counterpart. Expected action is to connect to the provided port.
			shutdown,						//Aria service is shutting down
			ping,							//Connection kept alive
		};



		///////////////
		// Connector //
		Connector::Connector(SocketTCP& _socket, const std::string& _name, const std::string& _counterpart, const protocol::PortType& _localPortForWaiting)
			: socket(_socket)
			, name(_name)
			, counterpart(_counterpart)
			, localPortForWaiting(_localPortForWaiting)
		{

		}

		bool Connector::enter(const protocol::Endpoint& endpoint)
		{
			if (socket.isConnected())
			{
				spehs::log::info("Aria::enter: Socket was already connected. Disconnecting before trying to enter...");
				socket.disconnect();
			}

			spehs::log::info("Aria::Connector::enter: connecting to aria...");
			if (socket.connect(endpoint))
			{
				//Send enter packet
				protocol::WriteBuffer buffer;
				buffer.write(ariaMagicHeader);
				buffer.write(PacketType::enter);
				buffer.write(name);
				buffer.write(counterpart);
				buffer.write(localPortForWaiting);
				enterResult = EnterResult::none;
				socket.sendPacket(buffer);

				//Start receiving
				socket.startReceiving(std::bind(&Connector::onReceive, this, std::placeholders::_1));

				//Prepare to ping				
				{
					std::lock_guard<std::mutex> lock(pingMutex);
					lastPingSendTime = spehs::time::now();
					lastPingReceiveTime = lastPingSendTime;
				}
				protocol::WriteBuffer pingBuffer;
				pingBuffer.write(ariaMagicHeader);
				pingBuffer.write(PacketType::ping);
				const spehs::time::Time pingTimeout = spehs::time::fromSeconds(10000.0f);

				//Wait for server to find a counterpart
				while (enterResult == EnterResult::none)
				{//Blocks

					socket.update();
					
					if (spehs::time::now() - lastPingSendTime > pingTimeout / 2)
					{//Send ping
						socket.sendPacket(pingBuffer);
						std::lock_guard<std::mutex> lock(pingMutex);
						lastPingSendTime = spehs::time::now();
					}

					std::lock_guard<std::mutex> lock(pingMutex);
					if (lastPingSendTime - lastPingReceiveTime >= pingTimeout)
					{//Ping timeout
						spehs::log::info("Aria::Connector::enter: failed. Connection to the remote server was lost (timeout).");
						enterResult = EnterResult::fail;
					}
				}

				//Disconnect from the aria server
				if (socket.isConnected())
					socket.disconnect(protocol::DisconnectType::doNotSendDisconnectPacket);

				if (enterResult == EnterResult::accept)
				{
					socket.startAccepting(localPortForWaiting, std::bind(&Connector::onAccept, this, std::placeholders::_1));
					const spehs::time::Time beginTime = spehs::time::now();
					spehs::log::info("Aria::Connector::enter: counterpart found. Awaiting for the counterpart to connect to this...");
					while (socket.isAccepting())
					{
						socket.update();

						//Blocks while accepting
						if (spehs::time::now() - beginTime >= pingTimeout)
						{
							socket.stopAccepting();
							spehs::log::info("Aria::Connector::enter: failed. A counterpart was found, but counterpart didn't connect to this within the given time.");
							while (socket.isAccepting()) {}
							return false;
						}
					}

					if (socket.isConnected())
					{
						spehs::log::info("Aria::Connector::enter: success.");
						return true;
					}
					else
					{
						spehs::log::info("Aria::Connector::enter: failed.");
						return false;
					}
				}
				else if (enterResult == EnterResult::connect)
				{
					spehs::log::info("Aria::Connector::enter: counterpart found. Connecting to the counterpart...");
					const spehs::time::Time beginTime = spehs::time::now();
					while (!socket.connect(connectEndpoint))
					{
						if (spehs::time::now() - beginTime >= pingTimeout)
						{
							spehs::log::info("Aria::Connector::enter: failed.");
							return false;
						}
					}
					SPEHS_ASSERT(socket.isConnected());
					spehs::log::info("Aria::Connector::enter: success.");
					return true;
				}
				else
					return false;
			}
			else
			{
				spehs::log::info("Aria::Connector::enter: Failed to connect to the provided aria endpoint.");
				return false;
			}
		}

		bool Connector::onReceive(protocol::ReadBuffer& buffer)
		{
			uint64_t headerVerification;
			buffer.read(headerVerification);
			if (headerVerification == ariaMagicHeader)
			{
				PacketType packetType;
				buffer.read(packetType);
				switch (packetType)
				{
				case PacketType::counterpartEndpointAccept:
					enterResult = EnterResult::accept;
					break;
				case PacketType::counterpartEndpointConnect:
					buffer.read(connectEndpoint.address);
					buffer.read(connectEndpoint.port);
					enterResult = EnterResult::connect;
					break;
				case PacketType::shutdown:
					spehs::log::info("Aria::Connector::enter: failed. The Aria server is shutting down...");
					enterResult = EnterResult::fail;
					break;
				case PacketType::ping:
				{
					std::lock_guard<std::mutex> lock(pingMutex);
					lastPingReceiveTime = spehs::time::now();
					spehs::log::info("Aria::Connector::enter: server ping: " + std::to_string((int)(lastPingReceiveTime - lastPingSendTime).asMilliseconds()) + " ms");
				}
					break;
				}

				return true;
			}
			else
			{
				socket.disconnect(protocol::DisconnectType::unknownProtocol);
				spehs::log::info("Aria::Connector::enter: failed. Remote endpoint responded with an unexpected response. Perhaps it is not an aria server, or an incompatible version.");
				return false;
			}
		}

		void Connector::onAccept(SocketTCP& socket)
		{
			//Nothing to do here
		}



		////////////
		// Client //
		static size_t nextClientId = 0;
		Client::Client(Server& _server)
			: server(_server)
			, socket(_server.ioService)
			, id(nextClientId++)
			, name()
			, counterpart()
			, localPortForWaiting(0)
		{

		}

		void Client::onAccept(SocketTCP& socket)
		{
			if (socket.isConnected())
			{
				//Send name and counterpart query
				protocol::WriteBuffer buffer;
				buffer.write(ariaMagicHeader);
				buffer.write(PacketType::enter);
				socket.sendPacket(buffer);

				//Start receiving
				socket.startReceiving(std::bind(&Client::onReceive, this, std::placeholders::_1));
			}
		}

		bool Client::onReceive(protocol::ReadBuffer& buffer)
		{
			uint64_t headerVerification;
			buffer.read(headerVerification);
			if (headerVerification == ariaMagicHeader)
			{
				PacketType packetType;
				buffer.read(packetType);
				switch (packetType)
				{
				case PacketType::enter:
				{//Received name, counterpart and wait port
					std::lock_guard<std::mutex> lock(enterDetailsMutex);
					enterDetailsChanged = true;
					buffer.read(name);
					buffer.read(counterpart);
					buffer.read(localPortForWaiting);
					spehs::log::info("Aria: client entered: '" + name + "', looking for counterpart: '" + counterpart + "'");
				}
				break;
				case PacketType::ping:
				{//Send a response
					static protocol::WriteBuffer pingBuffer;
					if (pingBuffer.getOffset() == 0)
					{
						pingBuffer.write(ariaMagicHeader);
						pingBuffer.write(PacketType::ping);
					}
					socket.sendPacket(pingBuffer);
				}
				break;
				}

				return true;
			}
			else
			{
				socket.disconnect(protocol::DisconnectType::unknownProtocol);
				return false;
			}
		}



		////////////
		// Server //
		Server::Server()
			: keepRunning(false)
			, canExitStart(false)
			, threadRunning(false)
			, thread(nullptr)
		{

		}

		Server::~Server()
		{
			//Stop running
			stop();
			while (isRunning()) {/*Blocks*/ }

			//Join thread
			std::lock_guard<std::recursive_mutex> lock(mutex);
			SPEHS_ASSERT(!keepRunning);
			if (thread)
			{
				thread->join();
				delete thread;
			}

			SPEHS_ASSERT(clients.empty());
		}

		void Server::start(const protocol::PortType& _localPort)
		{
			{
				std::lock_guard<std::recursive_mutex> lock(mutex);

				if (isRunning())
				{
					spehs::log::info("Aria is already running!");
					return;
				}

				if (thread)
				{//Wait for thread to fully exit
					thread->join();
					delete thread;
					thread = nullptr;
				}
				
				//Launch run thread
				spehs::log::info("Aria::start: starting at port '" + std::to_string(_localPort) + "'...");
				keepRunning = true;
				canExitStart = false;
				localPort = _localPort;
				thread = new std::thread(&Server::run, this);
			}

			//Blocks until run has started in a different thread
			while (true)
			{
				std::lock_guard<std::recursive_mutex> lock(mutex);
				if (canExitStart)
					break;
			}
		}

		void Server::stop()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			if (!thread || !keepRunning)
				return;

			//Request the run thread to exit
			keepRunning = false;
			return;
		}

		void Server::run()
		{
			//Set thread running variable
			RAIIMutexVariableSetter<bool, std::recursive_mutex> threadRunningSetter(threadRunning, true, mutex);

			{//Notify start() to be able to return
				std::lock_guard<std::recursive_mutex> lock(mutex);
				canExitStart = true;
			}

			//Run loop
			spehs::log::info("Aria::run: started.");
			while (true)
			{
				std::lock_guard<std::recursive_mutex> lock(mutex);

				//Process arrived packets
				for (size_t i = 0; i < clients.size(); i++)
					clients[i]->socket.update();

				//Make sure that one client is accepting
				int accepting = 0;
				for (size_t i = 0; i < clients.size(); i++)
				{
					if (clients[i]->socket.isAccepting())
						accepting++;
				}
				if (accepting == 0)
				{//Need to utilize an idle client for accepting, or launch a new one.
					Client* client = nullptr;
					for (size_t i = 0; i < clients.size(); i++)
					{
						if (!clients[i]->socket.isConnected())
						{
							client = clients[i];
							break;
						}
					}
					if (!client)
					{
						client = new Client(*this);
						clients.push_back(client);
					}
					std::lock_guard<std::mutex> lock(client->enterDetailsMutex);
					client->name.clear();
					client->counterpart.clear();
					client->socket.startAccepting(localPort, std::bind(&Client::onAccept, client, std::placeholders::_1));
				}
				else
				{
					SPEHS_ASSERT(accepting == 1);
				}

				//Remove idle clients
				for (size_t i = 0; i < clients.size();)
				{
					if (!clients[i]->socket.isAccepting() && !clients[i]->socket.isConnected())
					{
						spehs::log::info("Aria::run removing an idle client...");
						delete clients[i];
						clients[i] = clients.back();
						clients.pop_back();
					}
					else
						i++;
				}

				//Inspect possible pairings
				for (size_t c1 = 0; c1 < clients.size(); c1++)
				{
					clients[c1]->enterDetailsMutex.lock();
					const bool inspect = clients[c1]->enterDetailsChanged && clients[c1]->socket.isConnected();
					const std::string name1 = clients[c1]->name;
					const std::string counterpart1 = clients[c1]->counterpart;
					const protocol::PortType port1 = clients[c1]->localPortForWaiting;
					clients[c1]->enterDetailsChanged = false;
					clients[c1]->enterDetailsMutex.unlock();

					if (inspect)
					{//Check for a possible pair
						for (size_t c2 = 0; c2 < clients.size(); c2++)
						{
							if (c1 != c2 && clients[c2]->socket.isConnected())
							{
								clients[c2]->enterDetailsMutex.lock();
								const std::string name2 = clients[c2]->name;
								const std::string counterpart2 = clients[c2]->counterpart;
								clients[c2]->enterDetailsMutex.unlock();

								if (name1.size() > 0 && name2.size() > 0 && counterpart1 == name2 && counterpart2 == name1)
								{//Match
									spehs::log::info("Aria::run found a client pair: '" + name1 + "' & '" + name2 + "'");

									//c1 will accept
									protocol::WriteBuffer buffer1;
									buffer1.write(ariaMagicHeader);
									buffer1.write(PacketType::counterpartEndpointAccept);
									clients[c1]->socket.sendPacket(buffer1);

									//c2 will connect
									const protocol::AddressType address1 = clients[c1]->socket.getRemoteAddress();
									protocol::WriteBuffer buffer2;
									buffer2.write(ariaMagicHeader);
									buffer2.write(PacketType::counterpartEndpointConnect);
									buffer2.write(address1);
									buffer2.write(port1);
									clients[c2]->socket.sendPacket(buffer2);

									break;
								}
							}
						}
					}
				}

				if (!keepRunning)
					break;
			}

			{//End of run
				std::lock_guard<std::recursive_mutex> lock(mutex);
				//Deallocate clients
				for (size_t i = 0; i < clients.size(); i++)
					delete clients[i];
				clients.clear();
			}
		}

		bool Server::isRunning() const
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);
			return threadRunning;
		}
	}
}