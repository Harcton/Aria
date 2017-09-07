#pragma once
#include "IOService.h"
#include "SocketTCP.h"
#include "Protocol.h"
#include <atomic>

namespace codex
{
	/* A common shell interface */
	class Shell
	{
	public:
		Shell();
		virtual ~Shell();
		
		int start(const int argc, const char** argv);
		void stop();

		bool sendPacket(const protocol::WriteBuffer& buffer);

		/* Called once when starting */
		virtual void onStart() {}
		/* Called continously until stopped. */
		virtual void update() = 0;
		/* Called once when stopped. */
		virtual void onStop() {}

		/* Called when the shell receives a user defined packet. */
		virtual void receiveHandler(codex::protocol::ReadBuffer& buffer) {}

	private:
		bool internalReceiveHandler(codex::protocol::ReadBuffer& buffer);
		bool onAccept(codex::SocketTCP& socket);
		void run();
		IOService ioService;
		SocketTCP socket;
		std::atomic<bool> stopRequested;
	};
}