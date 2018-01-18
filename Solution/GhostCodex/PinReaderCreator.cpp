#include "SpehsEngine/Rendering/BatchManager.h"
#include "SpehsEngine/Rendering/LineDiagram.h"
#include "SpehsEngine/Rendering/Window.h"
#include "SpehsEngine/GUI/GUICheckbox.h"
#include "SpehsEngine/GUI/GUIStringEditor.h"
#include "SpehsEngine/GUI/GUIRectangleScrollList.h"
#include "Codex/Sync/SyncManager.h"
#include "PinReaderCreator.h"



namespace codex
{
	PinReaderCreator::Element::Element(spehs::GUIContext& context, const sync::Handle<codex::device::PinReaderGhost>& _handle)
		: spehs::GUIRectangleRow(context)
		, handle(_handle)
	{
		SPEHS_ASSERT(handle);
		addElement(remove = new spehs::GUIRectangle(context));
		remove->setString("x");
		addElement(active = new spehs::GUICheckbox(context));
		addElement(name = new spehs::GUIRectangle(context));
		name->setString(handle->name);
		lineDiagram = new spehs::LineDiagram2(context.batchManager);
		lineDiagram->setRenderState(false);
		lineDiagram->setSize(batchManager.window.getWidth(), batchManager.window.getHeight());
		lineDiagram->setCapacity(200);
		setDepth(getDepth());
	}

	PinReaderCreator::Element::~Element()
	{
		if (lineDiagram)
		{
			delete lineDiagram;
			lineDiagram = nullptr;
		}
	}

	void PinReaderCreator::Element::inputUpdate()
	{
		GUIRectangleRow::inputUpdate();

		SPEHS_ASSERT(handle);
		handle->setActive(active->getEditorValue());

		codex::device::PinReaderHistory history = handle->getHistory();
		handle->clearHistory();
		for (size_t i = 0; i < history.size(); i++)
		{
			if (history[i].state == gpio::PinState::high || history[i].state == gpio::PinState::low)
			{
				if (timeOrigin == spehs::time::zero)
					timeOrigin = history[i].time;
				const spehs::time::Time relativeToInit = history[i].time - timeOrigin;
				const spehs::vec2 point(relativeToInit.asMilliseconds(), history[i].state == gpio::PinState::high ? 1.0f : 0.0f);
				lineDiagram->pushBack(point);
			}
			else
				spehs::log::warning("PinReader history contains an invalid pin state");
		}
		
		lineDiagram->setRenderState(getRenderState() && getMouseHover());
	}

	void PinReaderCreator::Element::setRenderState(const bool state)
	{
		GUIRectangleRow::setRenderState(state);
		lineDiagram->setRenderState(state && getMouseHover());
	}

	void PinReaderCreator::Element::setDepth(const int16_t depth)
	{
		GUIRectangleRow::setDepth(depth);
		lineDiagram->setDepth(depth + 10);
	}

	PinReaderCreator::PinReaderCreator(spehs::GUIContext& context, sync::Manager& _syncManager)
		: GUIRectangleColumn(context)
		, syncManager(_syncManager)
	{
		addElement(nameEditor = new spehs::GUIStringEditor(context));
		nameEditor->setString("pin reader name");
		addElement(pinEditor = new spehs::GUIScalarEditor<unsigned>(context, "pin", 3));
		addElement(createButton = new spehs::GUIRectangle(context));
		createButton->setString("create");
		createButton->setJustification(GUIRECT_TEXT_JUSTIFICATION_CENTER_BIT);
		addElement(pinReaderList = new spehs::GUIRectangleScrollList(context));
	}

	PinReaderCreator::~PinReaderCreator()
	{

	}

	void PinReaderCreator::inputUpdate()
	{
		GUIRectangleColumn::inputUpdate();

		if (inputManager.isKeyPressed(MOUSEBUTTON_LEFT))
		{//Mouse button pressed

			if (createButton->getMouseHover())
				tryCreate();

			for (unsigned i = 0; i < pinReaderList->elementsSize(); i++)
			{
				Element* pinReader = (Element*)pinReaderList->at(i);
				if (pinReader->at(0/*remove*/)->getMouseHover())
				{
					pinReaderList->removeElement(pinReader);
				}
			}
		}
	}

	void PinReaderCreator::tryCreate()
	{
		const codex::gpio::Pin pin = codex::gpio::getPinNumberAsEnum(pinEditor->getEditorValue());
		if (pin == codex::gpio::Pin::pin_none)
		{
			spehs::log::info("Cannot create a pin reader with invalid pin.");
			return;
		}

		const std::string name = nameEditor->getEditorValue();
		if (name.empty())
		{
			spehs::log::info("Cannot create a pin reader without a name.");
			return;
		}

		for (size_t i = 0; i < pinReaderList->elementsSize(); i++)
		{
			if (pinReaderList->at(i)->getString() == name)
			{
				spehs::log::info("A pin reader with the name '" + name + "' has already been created.");
				return;
			}
		}

		sync::Handle<codex::device::PinReaderGhost> pinReaderHandle = syncManager.create<codex::device::PinReaderGhost>();
		if (pinReaderHandle)
		{
			//Servo configuration setup
			pinReaderHandle->name = name;
			pinReaderHandle->setActive(false);
			pinReaderHandle->setPin(pin);
			pinReaderHandle.setRemoteUpdateInterval(spehs::time::fromSeconds(1.0f));

			//Add element
			pinReaderList->addElement(new Element(getGUIContext(), pinReaderHandle));
		}
	}
}