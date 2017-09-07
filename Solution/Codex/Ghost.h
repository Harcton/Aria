#ifdef GHOST_CODEX
#pragma once
#include "IOService.h"
#include "SocketTCP.h"
#include "Protocol.h"
#include <atomic>

namespace codex
{
	/* A common ghost interface */
	class Ghost
	{
	public:
		Ghost();
		virtual ~Ghost();

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
		void run();
		IOService ioService;
		SocketTCP socket;
		std::atomic<bool> stopRequested;
	};
}
#endif