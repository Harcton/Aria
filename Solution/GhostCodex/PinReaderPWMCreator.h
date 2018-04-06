#pragma once
#include "SpehsEngine/GUI/GUIRectangleColumn.h"
#include "SpehsEngine/GUI/GUIRectangleRow.h"
#include "SpehsEngine/GUI/GUIScalarEditor.h"
#include "SpehsEngine/Sync/SyncHandle.h"
#include "SpehsEngine/GPIO/Device/PinReaderPWM.h"

namespace spehs
{
	class GUIStringEditor;
	class GUICheckbox;
	class LineDiagram;
	namespace sync
	{
		class Manager;
	}

	class PinReaderPWMCreator : public spehs::GUIRectangleColumn
	{
		class Element : public spehs::GUIRectangleRow
		{
		public:
			Element(spehs::GUIContext& context, const spehs::sync::Handle<spehs::device::PinReaderPWMGhost>& _handle);
			~Element();

			void inputUpdate() override;
			void setRenderState(const bool state) override;
			void setDepth(const int16_t depth) override;

			spehs::sync::Handle<spehs::device::PinReaderPWMGhost> handle;
		private:
			spehs::GUIRectangle* remove;
			spehs::GUICheckbox* active;
			spehs::GUIRectangle* name;
			spehs::LineDiagram* lineDiagram;
		};
	public:
		PinReaderPWMCreator(spehs::GUIContext& context, spehs::sync::Manager& _syncManager);
		~PinReaderPWMCreator();

		void inputUpdate() override;

		spehs::sync::Manager& syncManager;

	private:
		void tryCreate();

		spehs::GUIStringEditor* nameEditor;
		spehs::GUIScalarEditor<unsigned>* pinEditor;
		spehs::GUIScalarEditor<unsigned>* sampleRateEditor;
		spehs::GUIScalarEditor<unsigned>* sampleSizeEditor;
		spehs::GUIRectangle* createButton;
		spehs::GUIRectangleScrollList* pinReaderPWMList;
	};
}