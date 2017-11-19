#include <Codex/Codex.h>
#include <SpehsEngine/ApplicationData.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/AudioEngine.h>
#include <SpehsEngine/SpehsEngine.h>
#include <SpehsEngine/BatchManager.h>
#include <SpehsEngine/Camera2D.h>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/Time.h>
#include <SpehsEngine/RNG.h>

#include <Codex/SyncManager.h>
#include <Codex/Protocol.h>
#include <Codex/RTTI.h>

class MySyncType : public codex::SyncType
{
public:
	SYNCTYPE_DECL(MySyncType)
public:

	MySyncType()
	{

	}

	~MySyncType()
	{

	}
	void syncUpdate(codex::protocol::WriteBuffer& buffer) override;
	void syncUpdate(codex::protocol::ReadBuffer& buffer) override;
	
	int data1 = 1;
	int data2 = 2;
	int data3 = 3;
};
SYNCTYPE_IMPL(MySyncType)
void MySyncType::syncUpdate(codex::protocol::WriteBuffer& buffer)
{
	buffer.write(data1);
	buffer.write(data2);
	buffer.write(data3);
}
void MySyncType::syncUpdate(codex::protocol::ReadBuffer& buffer)
{
	buffer.read(data1);
	buffer.read(data2);
	buffer.read(data3);
}



/*
class MySyncType2 : public MySyncType
{
public:
	SYNCTYPE_DECL(MySyncType)
public:

	MySyncType2()
	{

	}
	~MySyncType2()
	{

	}
	void MySyncType::syncUpdate(codex::protocol::WriteBuffer& buffer) override;
	void MySyncType::syncUpdate(codex::protocol::ReadBuffer& buffer) override;

	int data4 = 4;
	int data5 = 5;
	int data6 = 6;
};
SYNCTYPE_IMPL(MySyncType)
void MySyncType2::syncUpdate(codex::protocol::WriteBuffer& buffer)
{

}
void MySyncType2::syncUpdate(codex::protocol::ReadBuffer& buffer)
{

}
*/





int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);
	spehs::initialize("Ghostbox");

	//Required class instances for spehs engine basic stuff
	spehs::Camera2D camera;
	spehs::BatchManager batchManager(&camera, "ghostbox");
	spehs::time::DeltaTimeSystem deltaTimeSystem;

	//Testing initialization here please...
	codex::IOService ioService;
	codex::SocketTCP socket(ioService);
	codex::SyncManager syncManager(socket);

	codex::protocol::Handshake h;
	codex::protocol::WriteBuffer wb;
	h.write(wb);
	codex::protocol::ReadBuffer rb(wb[0], wb.getCapacity());
	h.read(rb);

	syncManager.registerType<MySyncType>();
	//syncManager.registerType<MySyncType2>();
	syncManager.initialize();
	
	//Update & render loop
	bool run = true;
	while (run)
	{
		//Spehs update
		deltaTimeSystem.deltaTimeSystemUpdate();
		inputManager->update();
		spehs::audio::AudioEngine::update();
		spehs::console::update(deltaTimeSystem.deltaTime);
		if (inputManager->isQuitRequested() || inputManager->isKeyPressed(KEYBOARD_ESCAPE))
			run = false;

		//Test update...
		syncManager.update((codex::time::TimeType)deltaTimeSystem.deltaTime.value);
		
		//Render
		spehs::getMainWindow()->renderBegin();
		batchManager.render();
		spehs::console::render();
		spehs::getMainWindow()->renderEnd();
	}

	codex::uninitialize();
	return 0;
}