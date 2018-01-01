#pragma once
#include <SpehsEngine/GUI/GUIRectangleColumn.h>
#include <SpehsEngine/GUI/GUIRectangleRow.h>
#include <SpehsEngine/GUI/GUIScalarEditor.h>
namespace spehs
{
	class GUIStringEditor;
	class GUICheckbox;
}

namespace codex
{
	namespace sync
	{
		class Manager;
	}

	class ServoElement : public spehs::GUIRectangleRow
	{
	public:
		ServoElement(spehs::BatchManager& _batchManager, const sync::Handle<codex::device::ServoGhost>& _handle);

		void inputUpdate(InputUpdateData& data) override;

		sync::Handle<codex::device::ServoGhost> handle;

	private:
		spehs::GUIRectangle* remove;
		spehs::GUICheckbox* active;
		spehs::GUIRectangle* name;
		spehs::GUIScalarEditor<float>* angleEditor;
	};

	class ServoCreator : public spehs::GUIRectangleColumn
	{
	public:
		ServoCreator(spehs::BatchManager& _batchManager, sync::Manager& _syncManager);
		~ServoCreator();

		void inputUpdate(InputUpdateData& data) override;

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