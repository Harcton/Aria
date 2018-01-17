#include <SpehsEngine/GUI/GUICheckbox.h>
#include <SpehsEngine/GUI/GUIStringEditor.h>
#include <SpehsEngine/GUI/GUIRectangleScrollList.h>
#include <Codex/Device/Servo.h>
#include <Codex/Sync/SyncManager.h>
#include "ServoCreator.h"



namespace codex
{
	ServoElement::ServoElement(spehs::GUIContext& context, const sync::Handle<codex::device::ServoGhost>& _handle)
		: spehs::GUIRectangleRow(context)
		, handle(_handle)
	{
		SPEHS_ASSERT(handle);
		addElement(remove = new spehs::GUIRectangle(context));
		remove->setString("x");
		addElement(active = new spehs::GUICheckbox(context));
		addElement(name = new spehs::GUIRectangle(context));
		name->setString(handle->name);
		addElement(angleEditor = new spehs::GUIScalarEditor<float>(context, "angle", 0.0f));
		angleEditor->setMinValue(handle->getMinAngle());
		angleEditor->setMaxValue(handle->getMaxAngle());
	}

	void ServoElement::inputUpdate()
	{
		GUIRectangleRow::inputUpdate();

		SPEHS_ASSERT(handle);
		handle->setActive(active->getEditorValue());
		handle->setTargetAngle(angleEditor->getEditorValue());
	}



	ServoCreator::ServoCreator(spehs::GUIContext& context, sync::Manager& _syncManager)
		: GUIRectangleColumn(context)
		, syncManager(_syncManager)
	{
		addElement(nameEditor = new spehs::GUIStringEditor(context));
		nameEditor->setString("servo name");
		addElement(pinEditor = new spehs::GUIScalarEditor<unsigned>(context, "pin", 3));
		addElement(rotationSpeedEditor = new spehs::GUIScalarEditor<float>(context, "rotation speed", 5.0f));
		addElement(minAngleEditor = new spehs::GUIScalarEditor<float>(context, "min angle", 0.0f));
		addElement(maxAngleEditor = new spehs::GUIScalarEditor<float>(context, "max angle", 9.0f));
		addElement(minFrequencyEditor = new spehs::GUIScalarEditor<unsigned>(context, "min frequency (microseconds)", 1000));
		addElement(maxFrequencyEditor = new spehs::GUIScalarEditor<unsigned>(context, "max frequency (microseconds)", 2000));
		addElement(createButton = new spehs::GUIRectangle(context));
		createButton->setString("create");
		createButton->setJustification(GUIRECT_TEXT_JUSTIFICATION_CENTER_BIT);
		addElement(servoList = new spehs::GUIRectangleScrollList(context));
	}

	ServoCreator::~ServoCreator()
	{

	}

	void ServoCreator::inputUpdate()
	{
		GUIRectangleColumn::inputUpdate();

		if (inputManager.isKeyPressed(MOUSEBUTTON_LEFT))
		{//Mouse button pressed

			if (createButton->getMouseHover())
				tryCreate();

			for (unsigned i = 0; i < servoList->elementsSize(); i++)
			{
				ServoElement* servo = (ServoElement*)servoList->at(i);
				if (servo->at(0/*remove*/)->getMouseHover())
				{
					servoList->removeElement(servo);
				}
			}
		}
	}

	void ServoCreator::tryCreate()
	{
		const codex::gpio::Pin pin = codex::gpio::getPinNumberAsEnum(pinEditor->getEditorValue());
		if (pin == codex::gpio::Pin::pin_none)
		{
			spehs::log::info("Cannot create a servo with invalid pin.");
			return;
		}

		const std::string name = nameEditor->getEditorValue();
		if (name.empty())
		{
			spehs::log::info("Cannot create a servo without a name.");
			return;
		}

		for (size_t i = 0; i < servoList->elementsSize(); i++)
		{
			if (servoList->at(i)->getString() == name)
			{
				spehs::log::info("A servo with the name '" + name + "' has already been created.");
				return;
			}
		}
		
		sync::Handle<codex::device::ServoGhost> servoHandle = syncManager.create<codex::device::ServoGhost>();
		if (servoHandle)
		{
			//Servo configuration setup
			servoHandle->name = name;
			servoHandle->setActive(false);
			servoHandle->setPin(pin);
			servoHandle->setMinAngle(spehs::time::fromMicroseconds(minFrequencyEditor->getEditorValue()), minAngleEditor->getEditorValue());
			servoHandle->setMaxAngle(spehs::time::fromMicroseconds(maxFrequencyEditor->getEditorValue()), maxAngleEditor->getEditorValue());
			servoHandle->setRotationSpeed(rotationSpeedEditor->getEditorValue());

			//Add element
			servoList->addElement(new ServoElement(getGUIContext(), servoHandle));
		}
	}
}