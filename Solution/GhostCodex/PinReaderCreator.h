#pragma once
#include "SpehsEngine/GUI/GUIRectangleColumn.h"
#include "SpehsEngine/GUI/GUIRectangleRow.h"
#include "SpehsEngine/GUI/GUIScalarEditor.h"
#include "SpehsEngine/Sync/SyncHandle.h"
#include "SpehsEngine/GPIO/Device/PinReader.h"

namespace spehs
{
	class GUIStringEditor;
	class GUICheckbox;
	class LineDiagram;
	class LineDiagram2;
	namespace sync
	{
		class Manager;
	}

	class PinReaderCreator : public spehs::GUIRectangleColumn
	{
		class Element : public spehs::GUIRectangleRow
		{
		public:
			Element(spehs::GUIContext& context, const spehs::sync::Handle<spehs::device::PinReaderGhost>& _handle);
			~Element();

			void inputUpdate() override;
			void setRenderState(const bool state) override;
			void setDepth(const int16_t depth) override;

			spehs::sync::Handle<spehs::device::PinReaderGhost> handle;
		private:
			spehs::GUIRectangle* remove;
			spehs::GUICheckbox* active;
			spehs::GUIRectangle* name;
			spehs::LineDiagram2* lineDiagram;
			spehs::time::Time timeOrigin = 0;
		};
	public:
		PinReaderCreator(spehs::GUIContext& context, spehs::sync::Manager& _syncManager);
		~PinReaderCreator();

		void inputUpdate() override;

		spehs::sync::Manager& syncManager;

	private:
		void tryCreate();

		spehs::GUIStringEditor* nameEditor;
		spehs::GUIScalarEditor<unsigned>* pinEditor;
		spehs::GUIRectangle* createButton;
		spehs::GUIRectangleScrollList* pinReaderList;
	};
}