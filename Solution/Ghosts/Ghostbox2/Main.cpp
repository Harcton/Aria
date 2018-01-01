#include <Codex/Log.h>
#include <Codex/Codex.h>
#include <boost/system/error_code.hpp>//->Works
#include <Codex/Device/Servo.h>
#include <Codex/Sync/SyncManager.h>
#include <Codex/Aria.h>


int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);

	bool keepRunning = true;
	while (keepRunning)
	{
		codex::IOService ioService;
		codex::SocketTCP socket(ioService);
		codex::aria::Connector connector(socket, "arm0Shell", "arm0Ghost", 41623);
		if (connector.enter(codex::protocol::Endpoint("192.168.10.51", codex::protocol::defaultAriaPort)))
			codex::log::info("yay!");
		else
			codex::log::info("nay!");

		codex::sync::Manager syncManager(socket);
		syncManager.registerType<codex::device::ServoGhost, codex::device::ServoShell>();
		if (syncManager.initialize())
		{
			codex::time::TimeType deltaTime = 0;
			while (socket.isConnected())
			{
				const codex::time::TimeType beginTime = codex::time::now();
				socket.update();
				syncManager.update(deltaTime);
				deltaTime = codex::time::now() - beginTime;
			}
		}
	}

	codex::uninitialize();
	std::getchar();
	return 0;
}