#include <Codex/Codex.h>
#include <boost/system/error_code.hpp>//->Works
#include <Codex/Device/Servo.h>
#include <Codex/Device/PinReader.h>
#include <Codex/Device/PinReaderPWM.h>
#include <Codex/Sync/SyncManager.h>
#include <Codex/Aria.h>
#include <SpehsEngine/Core/Core.h>

int main(const int argc, const char** argv)
{
	spehs::CoreLib core;
	codex::initialize(argc, argv);

	bool keepRunning = true;
	while (keepRunning)
	{
		codex::IOService ioService;
		codex::SocketTCP socket(ioService);
		codex::aria::Connector connector(socket, "arm0Shell", "arm0Ghost", 41623);
		if (connector.enter(codex::protocol::Endpoint("192.168.10.51", codex::protocol::defaultAriaPort)))
			spehs::log::info("yay!");
		else
			spehs::log::info("nay!");

		codex::sync::Manager syncManager(socket);
		syncManager.registerType<codex::device::ServoShell>(codex::device::ServoGhost::getSyncTypeName(), codex::device::ServoGhost::getSyncTypeId(), codex::device::ServoGhost::getSyncTypeVersion());
		syncManager.registerType<codex::device::PinReaderShell>(codex::device::PinReaderGhost::getSyncTypeName(), codex::device::PinReaderGhost::getSyncTypeId(), codex::device::PinReaderGhost::getSyncTypeVersion());
		syncManager.registerType<codex::device::PinReaderPWMShell>(codex::device::PinReaderPWMGhost::getSyncTypeName(), codex::device::PinReaderPWMGhost::getSyncTypeId(), codex::device::PinReaderPWMGhost::getSyncTypeVersion());
		if (syncManager.initialize())
		{
			spehs::time::Time deltaTime = 0;
			while (socket.isConnected())
			{
				const spehs::time::Time beginTime = spehs::time::now();
				socket.update();
				syncManager.update(deltaTime);
				deltaTime = spehs::time::now() - beginTime;
			}
		}
	}

	codex::uninitialize();
	std::getchar();
	return 0;
}