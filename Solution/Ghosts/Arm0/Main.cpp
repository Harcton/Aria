//SPEHS ENGINE
#include <SpehsEngine/Input/InputManager.h>
#include <SpehsEngine/Audio/AudioEngine.h>
#include <SpehsEngine/Audio/Audio.h>
#include <SpehsEngine/Core/Core.h>
#include <SpehsEngine/Core/Inifile.h>
#include <SpehsEngine/Core/RNG.h>
#include <SpehsEngine/Core/Time.h>
#include <SpehsEngine/GUI/GUI.h>
#include <SpehsEngine/Input/Input.h>
#include <SpehsEngine/Rendering/BatchManager.h>
#include <SpehsEngine/Rendering/Camera2D.h>
#include <SpehsEngine/Rendering/Console.h>
#include <SpehsEngine/Rendering/GLContext.h>
#include <SpehsEngine/Rendering/Line.h>
#include <SpehsEngine/Rendering/Rendering.h>
#include <SpehsEngine/Rendering/ShaderManager.h>
#include <SpehsEngine/Rendering/Window.h>
#include <SpehsEngine/Rendering/TextureManager.h>
//CODEX
#include <Codex/Aria.h>
#include <Codex/Codex.h>
#include <Codex/IOService.h>
#include <Codex/SocketTCP.h>
#include <Codex/Manipulator.h>
#include <Codex/Device/Servo.h>
#include <Codex/Device/PinReader.h>
#include <Codex/Device/PinReaderPWM.h>
#include <Codex/Sync/SyncManager.h>
#include <Codex/Manipulator.h>
#include <GhostCodex/ServoCreator.h>
#include <GhostCodex/PinReaderCreator.h>
#include <GhostCodex/PinReaderPWMCreator.h>
//ARM0



int main(const int argc, const char** argv)
{
	//Spehs engine init
	spehs::CoreLib core;
	spehs::Inifile inifile("arm0");
	spehs::Inivar<unsigned>& windowWidth = inifile.get("Window", "width", 900u);
	spehs::Inivar<unsigned>& windowHeight = inifile.get("Window", "height", 600u);
	inifile.update();
	
	//Codex init
	codex::initialize(argc, argv);
	//Arm0 init
	spehs::time::delay(spehs::time::fromSeconds(1.0f));
	codex::IOService ioService;
	codex::SocketTCP socket(ioService);
	codex::aria::Connector connector(socket, "arm0Ghost", "arm0Shell", 41623);
	if (connector.enter(codex::protocol::Endpoint("192.168.10.51", codex::protocol::defaultAriaPort)))
		spehs::log::info("yay!");
	else
	{
		spehs::log::info("nay!");
		std::getchar();
		return 1;
	}
	
	spehs::AudioLib audio(core);
	spehs::RenderingLib rendering(core);
	spehs::InputLib input(rendering);
	spehs::GUILib gui(input, audio);
	spehs::Window window(windowWidth, windowHeight); window.setBorderless(true); window.setTitle("Arm0 ghost");
	spehs::InputManager inputManager(window);
	spehs::Camera2D camera(window);
	spehs::ShaderManager shaderManager;
	spehs::BatchManager batchManager(window, shaderManager, camera, "arm0");
	spehs::time::DeltaTimeSystem deltaTimeSystem;
	spehs::GUIContext guiContext(batchManager, inputManager, deltaTimeSystem);
	spehs::Console console;
	spehs::ConsoleVisualizer consoleVisualizer(console, inputManager, batchManager);

	codex::sync::Manager syncManager(socket);
	syncManager.registerType<codex::device::ServoGhost>(codex::device::ServoShell::getSyncTypeName(), codex::device::ServoShell::getSyncTypeId(), codex::device::ServoShell::getSyncTypeVersion());
	syncManager.registerType<codex::device::PinReaderGhost>(codex::device::PinReaderShell::getSyncTypeName(), codex::device::PinReaderShell::getSyncTypeId(), codex::device::PinReaderShell::getSyncTypeVersion());
	syncManager.registerType<codex::device::PinReaderPWMGhost>(codex::device::PinReaderPWMShell::getSyncTypeName(), codex::device::PinReaderPWMShell::getSyncTypeId(), codex::device::PinReaderPWMShell::getSyncTypeVersion());
	if (!syncManager.initialize())
	{
		std::getchar();
		return 1;
	}

	//Manipulator
	codex::Manipulator manipulator(syncManager);
	for (size_t i = 0; i < 6; i++)
		manipulator.pushBack();
	//Set manipulator transformation settings
	/*
	d1 = 0.0, 0.0
	d2 = 0.045, 0.05
	d3 = 0.0, 0.13
	d4 = -0.02, 0.023
	d5 = 0.145, 0.0
	d6 = 0.02, 0.0
	*/

	manipulator[0].setLocalPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	manipulator[0].setLocalAxis(glm::vec3(0.0f, 1.0f, 0.0f));
	manipulator[0].getServoGhost().setPin(codex::gpio::pin_18);
	manipulator[0].getServoGhost().setAngleLimits(spehs::time::fromMilliseconds(500), spehs::time::fromMilliseconds(2700), -PI * 0.5f, PI * 0.5f);
	manipulator[0].getServoGhost().setRotationSpeed(1.0f);
	manipulator[0].getServoGhost().setTargetAngle(0.0f);
	manipulator[0].getServoGhost().setActive(true);

	manipulator[1].setLocalPosition(glm::vec3(0.045f, 0.05f, 0.0f));
	manipulator[1].setLocalAxis(glm::vec3(0.0f, 0.0f, 1.0f));
	manipulator[1].getServoGhost().setPin(codex::gpio::pin_33);
	manipulator[1].getServoGhost().setAngleLimits(spehs::time::fromMilliseconds(1500), spehs::time::fromMilliseconds(2700), -PI * 0.5f, PI * 0.5f);
	manipulator[1].getServoGhost().setRotationSpeed(1.0f);
	manipulator[1].getServoGhost().setTargetAngle(0.0f);
	manipulator[1].getServoGhost().setActive(true);

	manipulator[2].setLocalPosition(glm::vec3(0.0f, 0.13f, 0.0f));
	manipulator[2].setLocalAxis(glm::vec3(0.0f, 0.0f, -1.0f));
	manipulator[2].getServoGhost().setPin(codex::gpio::pin_31);
	manipulator[2].getServoGhost().setAngleLimits(spehs::time::fromMilliseconds(1500), spehs::time::fromMilliseconds(2700), -PI * 0.5f, PI * 0.5f);
	manipulator[2].getServoGhost().setRotationSpeed(1.0f);
	manipulator[2].getServoGhost().setTargetAngle(0.0f);
	manipulator[2].getServoGhost().setActive(true);

	manipulator[3].setLocalPosition(glm::vec3(-0.02f, 0.023f, 0.0f));
	manipulator[3].setLocalAxis(glm::vec3(1.0f, 0.0f, 0.0f));
	manipulator[3].getServoGhost().setPin(codex::gpio::pin_40);
	manipulator[3].getServoGhost().setAngleLimits(spehs::time::fromMilliseconds(500), spehs::time::fromMilliseconds(2700), -PI * 0.5f, PI * 0.5f);
	manipulator[3].getServoGhost().setRotationSpeed(1.0f);
	manipulator[3].getServoGhost().setTargetAngle(0.0f);
	manipulator[3].getServoGhost().setActive(true);

	manipulator[4].setLocalPosition(glm::vec3(0.145f, 0.0f, 0.0f));
	manipulator[4].setLocalAxis(glm::vec3(1.0f, 0.0f, 0.0f));
	manipulator[4].getServoGhost().setPin(codex::gpio::pin_32);
	manipulator[4].getServoGhost().setAngleLimits(spehs::time::fromMilliseconds(500), spehs::time::fromMilliseconds(2250), -PI * 0.5f, PI * 0.5f);
	manipulator[4].getServoGhost().setRotationSpeed(1.0f);
	manipulator[4].getServoGhost().setTargetAngle(0.0f);
	manipulator[4].getServoGhost().setActive(true);

	manipulator[5].setLocalPosition(glm::vec3(0.02f, 0.0f, 0.0f));
	manipulator[5].setLocalAxis(glm::vec3(1.0f, 0.0f, 0.0f));
	manipulator[5].getServoGhost().setPin(codex::gpio::pin_29);
	manipulator[5].getServoGhost().setAngleLimits(spehs::time::fromMilliseconds(500), spehs::time::fromMilliseconds(2250), -PI * 0.5f, PI * 0.5f);
	manipulator[5].getServoGhost().setRotationSpeed(1.0f);
	manipulator[5].getServoGhost().setTargetAngle(0.0f);
	manipulator[5].getServoGhost().setActive(true);

	//Hierarchy lines
	std::vector<spehs::Line*> hierarchyLines;
	const int manipulatorSize = int(manipulator.size());
	for (int i = 0; i < manipulatorSize - 1; i++)
	{
		hierarchyLines.push_back(batchManager.createLine(0));
		hierarchyLines.back()->setCameraMatrixState(false);
		const float colorMultiplier = float(0.5f * (float(i) + manipulatorSize - 1)) / float(manipulatorSize - 1);
		hierarchyLines.back()->setColor(spehs::Color(int(255 * colorMultiplier), int(255 * colorMultiplier), int(255 * colorMultiplier)));
	}
	float cameraAngle = 0.0f;
	float visualScale = 1000.0f;
	const float visualScaleChangeSpeed = 10.0f;
	
	//Update & render loop
	bool run = true;
	const spehs::time::Time logTimeThreshold = spehs::time::fromMilliseconds(5);
	spehs::time::LapTimer lapTimer;
	while (run)
	{
		spehs::time::ScopeTimer cycleTimer;

		//Spehs update
		lapTimer.get();
		inifile.update();
		deltaTimeSystem.deltaTimeSystemUpdate();
		inputManager.update();
		spehs::audio::AudioEngine::update();
		consoleVisualizer.update(deltaTimeSystem.deltaTime);
		if (inputManager.isQuitRequested() || inputManager.isKeyPressed(KEYBOARD_ESCAPE))
			run = false;
		spehs::log::info("spehs update took " + std::to_string(lapTimer.get().asMilliseconds()) + " ms");

		//Test update...
		lapTimer.get();
		socket.update();
		spehs::log::info("socket update took " + std::to_string(lapTimer.get().asMilliseconds()) + " ms");
		lapTimer.get();
		syncManager.update(deltaTimeSystem.deltaTime);
		spehs::log::info("syncManager update took " + std::to_string(lapTimer.get().asMilliseconds()) + " ms");
		static float updateTargetAnglesTimer = 0.0f;
		updateTargetAnglesTimer -= deltaTimeSystem.deltaSeconds;
		if (updateTargetAnglesTimer <= 0.0f)
		{
			for (size_t i = 0; i < manipulator.size(); i++)
				manipulator[i].getServoGhost().setTargetAngle(spehs::rng::random<float>(manipulator[i].getServoGhost().getMinAngle(), manipulator[i].getServoGhost().getMaxAngle()));
			updateTargetAnglesTimer = 3.0f;
		}

		lapTimer.get();
		manipulator.update(deltaTimeSystem.deltaTime);
		spehs::log::info("manipulator update took " + std::to_string(lapTimer.get().asMilliseconds()) + " ms");

		lapTimer.get();
		const spehs::vec2 center(window.getWidth() / 2, window.getHeight() / 2);
		for (size_t i = 0; i < hierarchyLines.size(); i++)
		{
			const codex::ServoJoint& rj1 = manipulator[i];
			const codex::ServoJoint& rj2 = manipulator[i + 1];
			const glm::vec3 p1 = rj1.getGlobalPosition();
			const glm::vec3 p2 = rj2.getGlobalPosition();
			const float xFactor = cos(cameraAngle);
			const float zFactor = cos(cameraAngle + HALF_PI);
			const spehs::vec2 sp1(p1.x * xFactor + p1.y * zFactor, p1.y);
			const spehs::vec2 sp2(p2.x * xFactor + p2.y * zFactor, p2.y);
			hierarchyLines[i]->setPoints(center + visualScale * sp1, center + visualScale * sp2);
			hierarchyLines[i]->setLineWidth(0.2f * visualScale);
		}
		spehs::log::info("hierarchyLines update took " + std::to_string(lapTimer.get().asMilliseconds()) + " ms");

		//Render
		lapTimer.get();
		window.renderBegin();
		batchManager.render();
		consoleVisualizer.render();
		window.renderEnd();
		spehs::log::info("Rendering took " + std::to_string(lapTimer.get().asMilliseconds()) + " ms");
		spehs::log::info("Program cycle took " + std::to_string(cycleTimer.get().asMilliseconds()) + " ms");
	}

	codex::uninitialize();
}

/*
	Initialization
	//codex::ServoCreator servoCreator(guiContext, syncManager);
	//codex::PinReaderCreator pinReaderCreator(guiContext, syncManager);
	//codex::PinReaderPWMCreator pinReaderPWMCreator(guiContext, syncManager);

	Update
	//const float elementsHeight = servoCreator.getHeight() + pinReaderCreator.getHeight() + pinReaderPWMCreator.getHeight();
	//float penPositionY = (0.5f * window.getHeight()) - (0.5f * elementsHeight);
	////Servo creator
	//servoCreator.setPositionGlobal(window.getWidth() / 2 - servoCreator.getWidth() / 2, penPositionY);
	//penPositionY += servoCreator.getHeight();
	//servoCreator.inputUpdate();
	//servoCreator.visualUpdate();
	////Pin reader creator
	//pinReaderCreator.setPositionGlobal(window.getWidth() / 2 - pinReaderCreator.getWidth() / 2, penPositionY);
	//penPositionY += pinReaderCreator.getHeight();
	//pinReaderCreator.inputUpdate();
	//pinReaderCreator.visualUpdate();
	////Pin reader PWM creator
	//pinReaderPWMCreator.setPositionGlobal(window.getWidth() / 2 - pinReaderPWMCreator.getWidth() / 2, penPositionY);
	//penPositionY += pinReaderPWMCreator.getHeight();
	//pinReaderPWMCreator.inputUpdate();
	//pinReaderPWMCreator.visualUpdate();
*/