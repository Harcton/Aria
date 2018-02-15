#include <SpehsEngine/Audio/AudioEngine.h>
#include <SpehsEngine/Audio/Audio.h>
#include <SpehsEngine/Core/Core.h>
#include <SpehsEngine/Core/Inifile.h>
#include <SpehsEngine/Core/Time.h>
#include <SpehsEngine/Core/RNG.h>
#include <SpehsEngine/Input/Input.h>
#include <SpehsEngine/Input/InputManager.h>
#include <SpehsEngine/Rendering/Rendering.h>
#include <SpehsEngine/Rendering/Line.h>
#include <SpehsEngine/GUI/GUI.h>
#include <SpehsEngine/Rendering/TextureManager.h>
#include <SpehsEngine/Rendering/ShaderManager.h>
#include <SpehsEngine/Rendering/BatchManager.h>
#include <SpehsEngine/Rendering/Camera2D.h>
#include <SpehsEngine/Rendering/Polygon.h>
#include <SpehsEngine/Rendering/Console.h>
#include <SpehsEngine/Rendering/Window.h>
#include <SpehsEngine/Rendering/GLContext.h>
#include <SpehsEngine/GUI/GUIRectangle.h>
#include <Codex/Device/Servo.h>
#include <Codex/Sync/SyncManager.h>
#include <Codex/Manipulator.h>
#include <Codex/Protocol.h>
#include <Codex/Codex.h>
#include <Codex/Aria.h>
#include <thread>

//void runWindow(std::string windowName, spehs::Appvars& appvars)
void runWindow(std::string windowName, spehs::Inifile* ptr)
{
	spehs::Inifile& inifile = *ptr;
	spehs::Inivar<unsigned>& windowWidth = inifile.get(windowName, "width", 900u);
	spehs::Inivar<unsigned>& windowHeight = inifile.get(windowName, "height", 600u);

	spehs::Window window(windowWidth, windowHeight);
	window.setClearColor(spehs::Color(64, 0, 0));
	window.setTitle(windowName.c_str());
	spehs::Camera2D camera(window);
	spehs::ShaderManager shaderManager;
	spehs::BatchManager batchManager(window, shaderManager, camera, "ghostbox");
	spehs::InputManager inputManager(window);
	spehs::Console console;
	spehs::ConsoleVisualizer consoleVisualizer(console, inputManager, batchManager);
	spehs::time::DeltaTimeSystem deltaTimeSystem;
	spehs::GUIContext guiContext(batchManager, inputManager, deltaTimeSystem);

	//Testing initialization here please...
	std::vector<spehs::Polygon*> polygons;
	for (size_t i = 0; i < 24; i++)
	{
		console.log("log " + std::to_string(i));
	}
	
	//Update & render loop
	bool run = true;
	spehs::time::Time deltaTime = 0;
	while (run)
	{
		const spehs::time::Time beginTime = spehs::time::now();

		//Spehs update
		deltaTimeSystem.deltaTimeSystemUpdate();
		inputManager.update();
		spehs::audio::AudioEngine::update();
		consoleVisualizer.update(deltaTimeSystem.deltaTime);
		if (inputManager.isQuitRequested() || inputManager.isKeyPressed(KEYBOARD_ESCAPE))
			run = false;
		camera.update();
		
		//Test update...
		if (inputManager.isKeyDown(KEYBOARD_SPACE))
		{
			for (size_t i = 0; i < 10; i++)
			{
				polygons.push_back(batchManager.createPolygon(3, 0, 1.0f, 1.0f));
				polygons.back()->setCameraMatrixState(false);
				polygons.back()->setPosition(spehs::rng::random<float>(0.0f, (float)window.getWidth()), spehs::rng::random<float>(0.0f, (float)window.getHeight()));
			}
		}
		
		//Render
		window.renderBegin();
		batchManager.render();
		consoleVisualizer.render("FPS: " + std::to_string((int)(1.0f / deltaTime.asSeconds())));
		window.renderEnd();
		
		deltaTime = spehs::time::now() - beginTime;
	}
}

int main(const int argc, const char** argv)
{
	//Appvars
	spehs::Inifile inifile("ghostbox");
	inifile.read();

	//Initialize libraries
	spehs::CoreLib core;
	spehs::RenderingLib rendering(core);
	spehs::InputLib input(rendering);
	spehs::AudioLib audio(core);
	spehs::GUILib gui(rendering, audio);
	codex::initialize(argc, argv);

	const size_t threadCount = 1;
	std::thread* threads[threadCount];
	for (size_t i = 0; i < threadCount; i++)
	{
		const std::string str("Window" + std::to_string(i + 1));
		threads[i] = new std::thread(runWindow, str, &inifile);
	}

	spehs::time::delay(spehs::time::fromSeconds(1.0f));
	inifile.update();

	for (size_t i = 0; i < threadCount; i++)
	{
		threads[i]->join();
		delete threads[i];
	}

	codex::uninitialize();
	return 0;
}

/*


std::vector<codex::ServoJoint*> rotatingJoints;
for (size_t i = 0; i < 6; i++)
{
if (rotatingJoints.empty())
rotatingJoints.push_back(new codex::ServoJoint(nullptr));
else
{
rotatingJoints.push_back(new codex::ServoJoint(rotatingJoints.back()));
rotatingJoints.back()->localTranslate(glm::vec3(spehs::rng::random<float>(-1.0f, 1.0f), spehs::rng::random<float>(-1.0f, 1.0f), spehs::rng::random<float>(-1.0f, 1.0f)));
}
rotatingJoints.back()->localRotate(glm::quat(glm::vec3(spehs::rng::random<float>(-1.0f, 1.0f), spehs::rng::random<float>(-1.0f, 1.0f), spehs::rng::random<float>(-1.0f, 1.0f))));
}
std::vector<spehs::Line*> hierarchyLines;
for (int i = 0; i < (int)rotatingJoints.size() - 1; i++)
{
hierarchyLines.push_back(batchManager.createLine(0));
hierarchyLines.back()->setCameraMatrixState(false);
const float colorMultiplier = (float)i / float((int)rotatingJoints.size() - 1);
hierarchyLines.back()->setColor(spehs::Color(int(255 * colorMultiplier), int(255 * colorMultiplier), int(255 * colorMultiplier)));
}
float cameraAngle = 0.0f;
float visualScale = 100.0f;
const float visualScaleChangeSpeed = 10.0f;

//Update & render loop
bool run = true;
spehs::time::Time deltaTime = 0;
while (run)
{
const spehs::time::Time beginTime = spehs::time::now();

//Spehs update
deltaTimeSystem.deltaTimeSystemUpdate();
inputManager.update();
spehs::audio::AudioEngine::update();
consoleVisualizer.update(deltaTimeSystem.deltaTime);
if (inputManager.isQuitRequested() || inputManager.isKeyPressed(KEYBOARD_ESCAPE))
run = false;
camera.update();

//Test update...
if (inputManager.isKeyDown(KEYBOARD_SPACE))
{
for (size_t i = 0; i < 10; i++)
{
polygons.push_back(batchManager.createPolygon(3, 0, 1.0f, 1.0f));
polygons.back()->setCameraMatrixState(false);
polygons.back()->setPosition(spehs::rng::random<float>(0.0f, (float)window.getWidth()), spehs::rng::random<float>(0.0f, (float)window.getHeight()));
}
}
if (inputManager.isKeyDown(KEYBOARD_LEFT))
cameraAngle += deltaTime.asSeconds();
if (inputManager.isKeyDown(KEYBOARD_RIGHT))
cameraAngle -= deltaTime.asSeconds();
if (inputManager.isKeyDown(KEYBOARD_PAGEUP))
visualScale += visualScaleChangeSpeed * deltaTime.asSeconds();
if (inputManager.isKeyDown(KEYBOARD_PAGEDOWN))
visualScale = std::max(1.0f, visualScale - visualScaleChangeSpeed * deltaTime.asSeconds());
const spehs::vec2 center(window.getWidth() / 2, window.getHeight() / 2);
for (size_t i = 0; i < hierarchyLines.size(); i++)
{
codex::ServoJoint& rj1 = *rotatingJoints[i];
codex::ServoJoint& rj2 = *rotatingJoints[i + 1];
const glm::vec3 p1 = rj1.getGlobalPosition();
const glm::vec3 p2 = rj2.getGlobalPosition();
const float xFactor = cos(cameraAngle);
const float zFactor = cos(cameraAngle + HALF_PI);
const spehs::vec2 sp1(p1.x * xFactor + p1.y * zFactor, p1.y);
const spehs::vec2 sp2(p2.x * xFactor + p2.y * zFactor, p2.y);
hierarchyLines[i]->setPoints(center + visualScale * sp1, center + visualScale * sp2);
hierarchyLines[i]->setLineWidth(0.2f * visualScale);
}

//Render
window.renderBegin();
batchManager.render();
consoleVisualizer.render("FPS: " + std::to_string((int)(1.0f / deltaTime.asSeconds())));
window.renderEnd();

deltaTime = spehs::time::now() - beginTime;
}

*/