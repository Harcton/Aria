#pragma once
#include <SpehsEngine/Core/Vector.h>
#include <glm/gtc/quaternion.hpp>

namespace spehs
{
	namespace device
	{
		class ServoGhost;
	}
}
namespace spehs
{
	namespace time
	{
		struct Time;
	}
	class BatchManager;
	class Polygon;
	class Line;
}

class ServoVisualizer
{
public:
	ServoVisualizer(spehs::BatchManager& _batchManager, spehs::device::ServoGhost& _servo);
	~ServoVisualizer();
	
	void update(const spehs::time::Time& deltaTime);
	void render();

	void setParent(ServoVisualizer* _parent);
	void setRootScreenPosition(const spehs::vec2& pos);

	spehs::BatchManager& batchManager;
	spehs::device::ServoGhost& servo;

private:
	ServoVisualizer* parent;
	spehs::vec3 worldPosition;
	spehs::vec2 screenPosition;
	spehs::Line* parentLine;
};

