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

class MyGhostSyncType : public codex::GhostSyncType
{
public:
	GHOST_SYNCTYPE_DECL(MyGhostSyncType, 0)
public:

	~MyGhostSyncType() override {}

	void syncUpdateGhost(codex::protocol::WriteBuffer& buffer) override;
	void syncUpdateShell(codex::protocol::ReadBuffer& buffer) override;

	int data1 = 1;
	int data2 = 2;
	int data3 = 3;
};
void MyGhostSyncType::syncUpdateGhost(codex::protocol::WriteBuffer& buffer)
{
	buffer.write(data1);
	buffer.write(data2);
	buffer.write(data3);
}
void MyGhostSyncType::syncUpdateShell(codex::protocol::ReadBuffer& buffer)
{
	buffer.read(data1);
	buffer.read(data2);
	buffer.read(data3);
}
class MyShellSyncType : public codex::ShellSyncType
{
public:
	SHELL_SYNCTYPE_DECL(MyShellSyncType, 1)
public:

	~MyShellSyncType() override {}

	void syncUpdateGhost(codex::protocol::ReadBuffer& buffer) override;
	void syncUpdateShell(codex::protocol::WriteBuffer& buffer) override;

	int data1 = 1;
	int data2 = 2;
	int data3 = 3;
};
void MyShellSyncType::syncUpdateGhost(codex::protocol::ReadBuffer& buffer)
{
	buffer.read(data1);
	buffer.read(data2);
	buffer.read(data3);
}
void MyShellSyncType::syncUpdateShell(codex::protocol::WriteBuffer& buffer)
{
	buffer.write(data1);
	buffer.write(data2);
	buffer.write(data3);
}


//class MyGhostSyncType2 : public codex::GhostSyncType
//{
//public:
//	GHOST_SYNCTYPE_DECL(MyGhostSyncType2, 0)
//public:
//
//	~MyGhostSyncType2() override {}
//};
//class MyShellSyncType2 : public codex::ShellSyncType
//{
//public:
//	SHELL_SYNCTYPE_DECL(MyShellSyncType2, 0)
//public:
//
//	~MyShellSyncType2() override {}
//};






int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);
	spehs::initialize("Ghostbox");

	//Required class instances for spehs engine basic stuff
	spehs::Camera2D camera;
	spehs::BatchManager batchManager(&camera, "ghostbox");
	spehs::time::DeltaTimeSystem deltaTimeSystem;

	//Testing initialization here please...
	codex::SyncManager syncManager;
	syncManager.registerType<MyGhostSyncType, MyShellSyncType>();
	//syncManager.registerType<MyGhostSyncType2, MyShellSyncType2>();
	while (!syncManager.isConnected())
	{
		syncManager.startAccepting(49842);
		while (syncManager.isAccepting())
		{
			//Blocks
		}
		if (!syncManager.isConnected())
			codex::log::info("Sync manager failed to accept a connection, retrying...");
	}
	syncManager.initialize();
	MyGhostSyncType mySyncType;

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