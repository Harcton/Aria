#pragma once
#include "IOService.h"
#include "SocketTCP.h"
#include "Protocol.h"
#include <atomic>

namespace codex
{
	class Actor
	{
	public:
		Actor();
		virtual ~Actor();

		int start(const int argc, const char** argv);

		void stop();

		bool isConnected() const;
		bool isReceiving() const;
		bool isAccepting() const;

	protected:
		/* Called once when starting */
		virtual void onStart() {}
		/* Called continously until stopped. */
		virtual void update(const time::TimeType deltaTime) = 0;
		/* Called once when stopped. */
		virtual void onStop() {}
		/* Called when the actor receives a user defined packet. The user is responsible for mutex implementation. */
		virtual void receiveHandler(codex::protocol::ReadBuffer& buffer) {}

		void run();
		bool sendPacket(const protocol::WriteBuffer& buffer);
		void startReceiving();
		void stopReceiving();
		void startAccepting(const protocol::PortType& port);
		void stopAccepting();
		bool connect(const protocol::Endpoint& endpoint);

	private:
		bool internalReceiveHandler(codex::protocol::ReadBuffer& buffer);
		bool onAccept(codex::SocketTCP& socket);

		IOService ioService;
		SocketTCP socket;
		std::atomic<bool> stopRequested;
		std::vector<std::vector<unsigned char>> receivedPackets;//This vector is under the socket mutex protection!
	};
}