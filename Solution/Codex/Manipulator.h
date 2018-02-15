#pragma once
#include <vector>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Codex/Device/Servo.h"
#include "Codex/Sync/SyncHandle.h"

namespace codex
{
	namespace sync
	{
		class Manager;
	}
	
	class Manipulator;

	class ServoJoint
	{
		friend class Manipulator;
	public:

		ServoJoint(sync::Manager &syncManager, ServoJoint *parent);
		~ServoJoint();

		void update(const spehs::time::Time& deltaTime);

		//Root parent
		ServoJoint &getRootParent();
		const ServoJoint &getRootParent() const;

		//Parent
		ServoJoint *getParent();
		const ServoJoint *getParent() const;

		//Child
		ServoJoint *createChild();
		void removeChild();
		ServoJoint *getChild();
		const ServoJoint *getChild() const;
		
		//Local transform
		void setLocalPosition(const glm::vec3 &localPosition);
		void setLocalRotation(const glm::quat &localRotation);
		void setLocalAxis(const glm::vec3 &localAxis);
		void localTranslate(const glm::vec3 &localTranslation);
		void localRotate(const glm::quat &localRotation);
		glm::quat getLocalRotation() const;
		glm::vec3 getLocalPosition() const;
		glm::vec3 getLocalAxis() const;

		//Global transform
		void setGlobalPosition(const glm::vec3 &globalPosition);
		void setGlobalRotation(const glm::quat &globalRotation);
		void setGlobalAxis(const glm::vec3 &globalAxis);
		void globalTranslate(const glm::vec3 &globalTranslation);
		void globalRotate(const glm::quat &globalRotation);
		glm::quat getGlobalRotation() const;
		glm::vec3 getGlobalPosition() const;
		glm::vec3 getGlobalAxis() const;

		device::ServoGhost &getServoGhost();
		const device::ServoGhost &getServoGhost() const;

	private:

		sync::Manager &syncManager;
		sync::Handle<device::ServoGhost> servoGhostHandle;

		ServoJoint *parent = nullptr;
		ServoJoint *child = nullptr;

		//Transform offset (relative to parent)
		glm::quat rotation;
		glm::vec3 position;
		glm::vec3 axis;
	};

	/* Manipulator that works with only rotating joints */
	class Manipulator
	{
	public:
		Manipulator(sync::Manager &syncManager);
		~Manipulator();

		void update(const spehs::time::Time& deltaTime);

		ServoJoint &pushBack();

		ServoJoint &operator[](const size_t index);
		const ServoJoint &operator[](const size_t index) const;
		size_t size() const;

		sync::Manager &syncManager;

	private:
		ServoJoint root;
	};
}