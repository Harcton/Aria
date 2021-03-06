#pragma once
#include "SpehsEngine/GUI/GUIRectangleColumn.h"
#include "SpehsEngine/GUI/GUIRectangleRow.h"
#include "SpehsEngine/GUI/GUIScalarEditor.h"
#include "SpehsEngine/Sync/SyncHandle.h"
#include "SpehsEngine/GPIO/Device/Servo.h"

namespace spehs
{
	class GUIStringEditor;
	class GUICheckbox;
	namespace sync
	{
		class Manager;
	}

	class ServoElement : public spehs::GUIRectangleRow
	{
	public:
		ServoElement(spehs::GUIContext& context, const sync::Handle<spehs::device::ServoGhost>& _handle);

		void inputUpdate() override;

		sync::Handle<spehs::device::ServoGhost> handle;

	private:
		spehs::GUIRectangle* remove;
		spehs::GUICheckbox* active;
		spehs::GUIRectangle* name;
		spehs::GUIScalarEditor<float>* angleEditor;
	};

	class ServoCreator : public spehs::GUIRectangleColumn
	{
	public:
		ServoCreator(spehs::GUIContext& context, sync::Manager& _syncManager);
		~ServoCreator();

		void inputUpdate() override;

		sync::Manager& syncManager;

	private:
		void tryCreate();

		spehs::GUIStringEditor* nameEditor;
		spehs::GUIScalarEditor<unsigned>* pinEditor;
		spehs::GUIScalarEditor<unsigned>* minFrequencyEditor;
		spehs::GUIScalarEditor<float>* minAngleEditor;
		spehs::GUIScalarEditor<unsigned>* maxFrequencyEditor;
		spehs::GUIScalarEditor<float>* maxAngleEditor;
		spehs::GUIScalarEditor<float>* rotationSpeedEditor;
		spehs::GUIRectangle* createButton;
		spehs::GUIRectangleScrollList* servoList;
	};
}