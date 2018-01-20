#include "SpehsEngine/Rendering/BatchManager.h"
#include "SpehsEngine/Rendering/LineDiagram.h"
#include "SpehsEngine/Rendering/Window.h"
#include "SpehsEngine/GUI/GUICheckbox.h"
#include "SpehsEngine/GUI/GUIStringEditor.h"
#include "SpehsEngine/GUI/GUIRectangleScrollList.h"
#include "Codex/Sync/SyncManager.h"
#include "PinReaderPWMCreator.h"



namespace codex
{
	PinReaderPWMCreator::Element::Element(spehs::GUIContext& context, const sync::Handle<codex::device::PinReaderPWMGhost>& _handle)
		: spehs::GUIRectangleRow(context)
		, handle(_handle)
	{
		SPEHS_ASSERT(handle);
		addElement(remove = new spehs::GUIRectangle(context));
		remove->setString("x");
		addElement(active = new spehs::GUICheckbox(context));
		addElement(name = new spehs::GUIRectangle(context));
		name->setString(handle->name);
		lineDiagram = new spehs::LineDiagram(context.batchManager);
		lineDiagram->setRenderState(false);
		lineDiagram->setSize(batchManager.window.getWidth(), batchManager.window.getHeight());
		lineDiagram->setCapacity(200);
		setDepth(getDepth());
	}

	PinReaderPWMCreator::Element::~Element()
	{
		if (lineDiagram)
		{
			delete lineDiagram;
			lineDiagram = nullptr;
		}
	}

	void PinReaderPWMCreator::Element::inputUpdate()
	{
		GUIRectangleRow::inputUpdate();

		SPEHS_ASSERT(handle);
		handle->setActive(active->getEditorValue());

		codex::device::PWMHistory history = handle->getHistory();
		handle->clearHistory();
		for (size_t i = 0; i < history.size(); i++)
		{
			if (history[i] == gpio::PinState::high)
				lineDiagram->pushBack(1.0f);
			else if (history[i] == gpio::PinState::low)
				lineDiagram->pushBack(0.0f);
		}
		
		lineDiagram->setRenderState(getRenderState() && getMouseHover());
	}

	void PinReaderPWMCreator::Element::setRenderState(const bool state)
	{
		GUIRectangleRow::setRenderState(state);
		lineDiagram->setRenderState(state && getMouseHover());
	}

	void PinReaderPWMCreator::Element::setDepth(const int16_t depth)
	{
		GUIRectangleRow::setDepth(depth);
		lineDiagram->setDepth(depth + 10);
	}

	PinReaderPWMCreator::PinReaderPWMCreator(spehs::GUIContext& context, sync::Manager& _syncManager)
		: GUIRectangleColumn(context)
		, syncManager(_syncManager)
	{
		addElement(nameEditor = new spehs::GUIStringEditor(context));
		nameEditor->setString("pin reader PWM name");
		addElement(pinEditor = new spehs::GUIScalarEditor<unsigned>(context, "pin", 3));
		addElement(createButton = new spehs::GUIRectangle(context));
		createButton->setString("create");
		createButton->setJustification(GUIRECT_TEXT_JUSTIFICATION_CENTER_BIT);
		addElement(sampleRateEditor = new spehs::GUIScalarEditor<unsigned>(context, "sample rate / s", 1000));
		addElement(sampleSizeEditor = new spehs::GUIScalarEditor<unsigned>(context, "sample size / value", 100));
		addElement(pinReaderPWMList = new spehs::GUIRectangleScrollList(context));
	}

	PinReaderPWMCreator::~PinReaderPWMCreator()
	{

	}

	void PinReaderPWMCreator::inputUpdate()
	{
		GUIRectangleColumn::inputUpdate();

		if (inputManager.isKeyPressed(MOUSEBUTTON_LEFT))
		{//Mouse button pressed

			if (createButton->getMouseHover())
				tryCreate();

			for (unsigned i = 0; i < pinReaderPWMList->elementsSize(); i++)
			{
				Element* pinReader = (Element*)pinReaderPWMList->at(i);
				if (pinReader->at(0/*remove*/)->getMouseHover())
				{
					pinReaderPWMList->removeElement(pinReader);
				}
			}
		}
	}

	void PinReaderPWMCreator::tryCreate()
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

		for (size_t i = 0; i < pinReaderPWMList->elementsSize(); i++)
		{
			if (pinReaderPWMList->at(i)->getString() == name)
			{
				spehs::log::info("A pin reader with the name '" + name + "' has already been created.");
				return;
			}
		}

		sync::Handle<codex::device::PinReaderPWMGhost> pinReaderHandle = syncManager.create<codex::device::PinReaderPWMGhost>();
		if (pinReaderHandle)
		{
			//Servo configuration setup
			pinReaderHandle.setRemoteUpdateInterval(spehs::time::fromSeconds(1.0f / 10.0f));
			pinReaderHandle->name = name;
			pinReaderHandle->setActive(false);
			pinReaderHandle->setPin(pin);
			pinReaderHandle->setSampleSize(sampleSizeEditor->getEditorValue());
			pinReaderHandle->setSampleRate(spehs::time::fromSeconds(1.0f / (float)sampleRateEditor->getEditorValue()));

			//Add element
			pinReaderPWMList->addElement(new Element(getGUIContext(), pinReaderHandle));
		}
	}
}