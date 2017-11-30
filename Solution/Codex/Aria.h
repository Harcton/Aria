#pragma once
#include <stdint.h>
#include <thread>
#include <mutex>
#include "Protocol.h"
#include "IOService.h"
#include "SocketTCP.h"



namespace codex
{
	class SocketTCP;

	namespace aria
	{
		class Server;

		class Connector
		{
		public:
			Connector(SocketTCP& socket, const std::string& name, const std::string& counterpart, const protocol::PortType& _localPortForWaiting);

			bool enter(const protocol::Endpoint& endpoint);//TODO: , const time::TimeType timeout = 0
			bool onReceive(protocol::ReadBuffer& buffer);
			void onAccept(SocketTCP& socket);

			SocketTCP& socket;
			const std::string name;
			const std::string counterpart;
			const protocol::PortType localPortForWaiting;

		private:
			enum EnterResult { none, connect, accept, fail };
			std::atomic<EnterResult> enterResult;
			protocol::Endpoint connectEndpoint;

			std::mutex pingMutex;
			codex::time::TimeType lastPingSendTime;
			codex::time::TimeType lastPingReceiveTime;
		};

		class Client
		{
		public:
			Client(Server& _server);
			void onAccept(SocketTCP& socket);
			bool onReceive(protocol::ReadBuffer& buffer);

			Server& server;
			SocketTCP socket;
			const size_t id;

			std::mutex enterDetailsMutex;
			bool enterDetailsChanged;
			std::string name;
			std::string counterpart;
			protocol::PortType localPortForWaiting;
		};

		/* Aria server instance, thread-safe */
		class Server
		{
			/*
			Connects socket to an aria server.
			Blocks until counterpart is found, or until timeout.
			Returns true on success, false on failure.
			*/
			friend class Client;
		public:
		public:

			Server();
			~Server();

			/* Hosts an aria server on a local port. */
			void start(const protocol::PortType& localPort);

			/* Stops hosting. */
			void stop();

			bool isRunning() const;

		private:
			void run();

			mutable std::recursive_mutex mutex;

			//Run state
			std::thread* thread;
			bool keepRunning;
			bool canExitStart;
			bool threadRunning;
			protocol::PortType localPort;

			//Clients
			IOService ioService;
			std::vector<Client*> clients;
		};
	}
}