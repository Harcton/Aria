#include <boost/system/error_code.hpp>//->Works
#include "SpehsEngine/GPIO/GPIO.h"
#include "SpehsEngine/GPIO/Device/Servo.h"
#include "SpehsEngine/GPIO/Device/PinReader.h"
#include "SpehsEngine/GPIO/Device/PinReaderPWM.h"
#include "SpehsEngine/Sync/SyncManager.h"
#include "SpehsEngine/Sync/Sync.h"
#include "SpehsEngine/Net/Net.h"
#include "SpehsEngine/Net/Aria.h"
#include "SpehsEngine/Core/Core.h"

int main(const int argc, const char** argv)
{
	spehs::CoreLib core;
	spehs::NetLib netLib(core);
	spehs::SyncLib syncLib(core);
	spehs::GPIOLib gpioLib(syncLib);

	bool keepRunning = true;
	while (keepRunning)
	{
		spehs::IOService ioService;
		spehs::SocketTCP socket(ioService);
		spehs::aria::Connector connector(socket, "arm0Shell", "arm0Ghost", 41623);
		if (connector.enter(spehs::net::Endpoint("192.168.10.51", spehs::net::defaultAriaPort)))
			spehs::log::info("yay!");
		else
			spehs::log::info("nay!");

		spehs::sync::Manager syncManager(socket);
		syncManager.registerType<spehs::device::ServoShell>(spehs::device::ServoGhost::getSyncTypeName(), spehs::device::ServoGhost::getSyncTypeId(), spehs::device::ServoGhost::getSyncTypeVersion());
		syncManager.registerType<spehs::device::PinReaderShell>(spehs::device::PinReaderGhost::getSyncTypeName(), spehs::device::PinReaderGhost::getSyncTypeId(), spehs::device::PinReaderGhost::getSyncTypeVersion());
		syncManager.registerType<spehs::device::PinReaderPWMShell>(spehs::device::PinReaderPWMGhost::getSyncTypeName(), spehs::device::PinReaderPWMGhost::getSyncTypeId(), spehs::device::PinReaderPWMGhost::getSyncTypeVersion());
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

	std::getchar();
	return 0;
}