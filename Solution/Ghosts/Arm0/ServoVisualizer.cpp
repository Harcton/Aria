#include "ServoVisualizer.h"
#include <SpehsEngine/Rendering/BatchManager.h>
#include <SpehsEngine/Core/Time.h>
#include <Codex/Device/Servo.h>



ServoVisualizer::ServoVisualizer(spehs::BatchManager& _batchManager, codex::device::ServoGhost& _servo)
	: batchManager(_batchManager)
	, servo(_servo)
	, parent(nullptr)
{

}

ServoVisualizer::~ServoVisualizer()
{

}

void ServoVisualizer::setParent(ServoVisualizer* _parent)
{
	parent = _parent;
}

void ServoVisualizer::setRootScreenPosition(const spehs::vec2& pos)
{
	if (parent)
		parent->setRootScreenPosition(pos);
	else
		screenPosition = pos;
}

void ServoVisualizer::update(const spehs::time::Time& deltaTime)
{

}

void ServoVisualizer::render()
{

}