#include "Manipulator.h"
#include "Codex/Sync/SyncManager.h"



namespace codex
{
	ServoJoint::ServoJoint(sync::Manager &syncManagerParam, ServoJoint *parentParam)
		: parent(parentParam)
		, rotation(glm::mat4(1.0))//Identity
		, syncManager(syncManagerParam)
		, servoGhostHandle(syncManager.create<device::ServoGhost>())
	{
		SPEHS_ASSERT(servoGhostHandle);
	}

	ServoJoint::~ServoJoint()
	{
		if (child)
		{
			delete child;
		}
	}

	void ServoJoint::update(const spehs::time::Time &deltaTime)
	{
		setLocalRotation(glm::quat(servoGhostHandle->getApproximatedAngle(), getLocalAxis()));
	}
	
	ServoJoint& ServoJoint::getRootParent()
	{
		if (parent)
			return parent->getRootParent();
		else
			return *this;
	}

	const ServoJoint& ServoJoint::getRootParent() const
	{
		if (parent)
			return parent->getRootParent();
		else
			return *this;
	}

	ServoJoint *ServoJoint::getParent()
	{
		return parent;
	}

	const ServoJoint *ServoJoint::getParent() const
	{
		return parent;
	}

	ServoJoint *ServoJoint::getChild()
	{
		return child;
	}

	const ServoJoint *ServoJoint::getChild() const
	{
		return child;
	}
	
	void ServoJoint::setLocalPosition(const glm::vec3& localPosition)
	{
		position = localPosition;
	}

	void ServoJoint::setLocalRotation(const glm::quat& localRotation)
	{
		rotation = localRotation;
	}

	void ServoJoint::setLocalAxis(const glm::vec3& localAxis)
	{
		axis = glm::normalize(localAxis);
	}

	void ServoJoint::localTranslate(const glm::vec3& translation)
	{
		position += translation;
	}

	void ServoJoint::localRotate(const glm::quat& rotate)
	{		
		rotation = glm::normalize(rotation * rotate);
	}

	glm::vec3 ServoJoint::getLocalPosition() const
	{
		return position;
	}

	glm::quat ServoJoint::getLocalRotation() const
	{
		return rotation;
	}

	glm::vec3 ServoJoint::getLocalAxis() const
	{
		return axis;
	}

	void ServoJoint::setGlobalPosition(const glm::vec3 &targetGlobalPosition)
	{
		const glm::vec3 currentGlobalPosition = getGlobalPosition();
		const glm::vec3 globalTranslation = targetGlobalPosition - currentGlobalPosition;
		globalTranslate(globalTranslation);
	}

	void ServoJoint::setGlobalRotation(const glm::quat &targetGlobalRotation)
	{
		const glm::quat currentGlobalRotation = getGlobalRotation();
		const glm::quat globalRotation = glm::normalize(glm::inverse(currentGlobalRotation) * targetGlobalRotation);
		globalRotate(globalRotation);
	}

	void ServoJoint::setGlobalAxis(const glm::vec3& globalAxis)
	{
		if (parent)
		{
			const glm::quat parentGlobalRotation = parent->getGlobalRotation();
			const glm::quat inverseParentGlobalRotation = glm::inverse(parentGlobalRotation);
			axis += glm::normalize(inverseParentGlobalRotation * globalAxis);
		}
		else
		{
			axis = glm::normalize(globalAxis);
		}
	}

	void ServoJoint::globalTranslate(const glm::vec3& translation)
	{
		if (parent)
		{
			const glm::quat parentGlobalRotation = parent->getGlobalRotation();
			const glm::quat inverseParentGlobalRotation = glm::inverse(parentGlobalRotation);
			position += inverseParentGlobalRotation * translation;
		}
		else
		{
			position += translation;
		}
	}

	void ServoJoint::globalRotate(const glm::quat& rotate)
	{
		if (parent)
		{
			const glm::quat parentGlobalRotation = parent->getGlobalRotation();
			const glm::quat inverseParentGlobalRotation = glm::inverse(parentGlobalRotation);
			rotation = glm::normalize(inverseParentGlobalRotation * rotate * rotation);
		}
		else
		{
			rotation = glm::normalize(rotation * rotate);
		}
	}

	glm::vec3 ServoJoint::getGlobalPosition() const
	{
		if (parent)
		{
			//Must account parent's global rotation
			const glm::quat parentGlobalRotation = parent->getGlobalRotation();
			return parent->getGlobalPosition() + position * parentGlobalRotation;
		}
		else
		{
			return position;
		}
	}

	glm::quat ServoJoint::getGlobalRotation() const
	{
		if (parent)
		{
			return parent->getGlobalRotation();
		}
		else
		{
			return rotation;
		}
	}

	glm::vec3 ServoJoint::getGlobalAxis() const
	{
		if (parent)
		{
			const glm::quat parentGlobalRotation = parent->getGlobalRotation();
			return axis * parentGlobalRotation;
		}
		else
		{
			return axis;
		}
	}

	device::ServoGhost &ServoJoint::getServoGhost()
	{
		return *servoGhostHandle;
	}

	const device::ServoGhost &ServoJoint::getServoGhost() const
	{
		return *servoGhostHandle;
	}

	ServoJoint *ServoJoint::createChild()
	{
		if (child)
		{
			SPEHS_ASSERT(false && "Child already exists");
			return nullptr;
		}
		child = new ServoJoint(syncManager, this);
		return child;
	}

	void ServoJoint::removeChild()
	{
		SPEHS_ASSERT(child);
		delete child;
		child = nullptr;
	}

	Manipulator::Manipulator(sync::Manager& syncManagerParam)
		: syncManager(syncManagerParam)
		, root(syncManagerParam, nullptr)
	{
		syncManager.create<device::ServoGhost>();
	}
	
	Manipulator::~Manipulator()
	{

	}

	void Manipulator::update(const spehs::time::Time &deltaTime)
	{
		ServoJoint* servoJoint = &root;
		while (servoJoint)
		{
			servoJoint->update(deltaTime);
			servoJoint = servoJoint->child;
		}
	}

	ServoJoint &Manipulator::pushBack()
	{
		ServoJoint *joint = &root;
		while (joint->getChild())
		{
			joint = joint->getChild();
		}
		ServoJoint *child = joint->createChild();
		SPEHS_ASSERT(child);
		return *child;
	}

	ServoJoint &Manipulator::operator[](const size_t targetIndex)
	{
		ServoJoint* servoJoint = &root;
		size_t currentIndex = 0;
		while (currentIndex++ < targetIndex)
		{
			SPEHS_ASSERT(servoJoint->child);
			if (!servoJoint->child)
				spehs::log::error("Manipulator::operator[] index out of range");
			servoJoint = servoJoint->child;
		}
		return *servoJoint;
	}

	const ServoJoint &Manipulator::operator[](const size_t targetIndex) const
	{
		const ServoJoint* servoJoint = &root;
		size_t currentIndex = 0;
		while (currentIndex++ < targetIndex)
		{
			SPEHS_ASSERT(servoJoint->child);
			if (!servoJoint->child)
				spehs::log::error("const Manipulator::operator[] index out of range");
			servoJoint = servoJoint->child;
		}
		return *servoJoint;
	}

	size_t Manipulator::size() const
	{
		const ServoJoint* servoJoint = &root;
		size_t size = 0;
		while (servoJoint)
		{
			size++;
			servoJoint = servoJoint->child;
		}
		return size;
	}
}