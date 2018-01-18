#pragma once
#include "SpehsEngine/GUI/GUIRectangleColumn.h"
#include "SpehsEngine/GUI/GUIRectangleRow.h"
#include "SpehsEngine/GUI/GUIScalarEditor.h"
#include "Codex/Sync/SyncHandle.h"
#include "Codex/Device/PinReader.h"

namespace spehs
{
	class GUIStringEditor;
	class GUICheckbox;
	class LineDiagram;
	class LineDiagram2;
}

namespace codex
{
	namespace sync
	{
		class Manager;
	}

	class PinReaderCreator : public spehs::GUIRectangleColumn
	{
		class Element : public spehs::GUIRectangleRow
		{
		public:
			Element(spehs::GUIContext& context, const sync::Handle<codex::device::PinReaderGhost>& _handle);
			~Element();

			void inputUpdate() override;
			void setRenderState(const bool state) override;
			void setDepth(const int16_t depth) override;

			sync::Handle<codex::device::PinReaderGhost> handle;
		private:
			spehs::GUIRectangle* remove;
			spehs::GUICheckbox* active;
			spehs::GUIRectangle* name;
			spehs::LineDiagram2* lineDiagram;
			spehs::time::Time timeOrigin = 0;
		};
	public:
		PinReaderCreator(spehs::GUIContext& context, sync::Manager& _syncManager);
		~PinReaderCreator();

		void inputUpdate() override;

		sync::Manager& syncManager;

	private:
		void tryCreate();

		spehs::GUIStringEditor* nameEditor;
		spehs::GUIScalarEditor<unsigned>* pinEditor;
		spehs::GUIRectangle* createButton;
		spehs::GUIRectangleScrollList* pinReaderList;
	};
}