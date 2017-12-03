#include <SpehsEngine/GUICheckbox.h>
#include <SpehsEngine/GUIStringEditor.h>
#include <SpehsEngine/GUIRectangleScrollList.h>
#include <Codex/Device/Servo.h>
#include <Codex/Sync/SyncManager.h>
#include "ServoCreator.h"



namespace codex
{
	ServoElement::ServoElement(spehs::BatchManager& _batchManager, const sync::Handle<codex::device::ServoGhost>& _handle)
		: spehs::GUIRectangleRow(_batchManager)
		, handle(_handle)
	{
		CODEX_ASSERT(handle);
		addElement(remove = new spehs::GUIRectangle(_batchManager));
		remove->setString("x");
		addElement(active = new spehs::GUICheckbox(_batchManager));
		addElement(name = new spehs::GUIRectangle(_batchManager));
		name->setString(handle->name);
		addElement(angleEditor = new spehs::GUIScalarEditor<float>(_batchManager, "angle", 0.0f));
		angleEditor->setMinValue(handle->getMinAngle());
		angleEditor->setMaxValue(handle->getMaxAngle());
	}

	void ServoElement::inputUpdate(InputUpdateData& data)
	{
		GUIRectangleRow::inputUpdate(data);

		CODEX_ASSERT(handle);
		handle->setActive(active->getEditorValue());
		handle->setTargetAngle(angleEditor->getEditorValue());
	}



	ServoCreator::ServoCreator(spehs::BatchManager& _batchManager, sync::Manager& _syncManager)
		: GUIRectangleColumn(_batchManager)
		, syncManager(_syncManager)
	{
		addElement(nameEditor = new spehs::GUIStringEditor(_batchManager));
		nameEditor->setString("servo name");
		addElement(pinEditor = new spehs::GUIScalarEditor<unsigned>(_batchManager, "pin", 3));
		addElement(rotationSpeedEditor = new spehs::GUIScalarEditor<float>(_batchManager, "rotation speed", 0.0f));
		addElement(minAngleEditor = new spehs::GUIScalarEditor<float>(_batchManager, "min angle", -180.0f));
		addElement(maxAngleEditor = new spehs::GUIScalarEditor<float>(_batchManager, "max angle", 180.0f));
		addElement(minFrequencyEditor = new spehs::GUIScalarEditor<unsigned>(_batchManager, "min frequency (micro seconds)", 1000));
		addElement(maxFrequencyEditor = new spehs::GUIScalarEditor<unsigned>(_batchManager, "max frequency (micro seconds)", 2000));
		addElement(createButton = new spehs::GUIRectangle(_batchManager));
		createButton->setString("create");
		createButton->setJustification(GUIRECT_TEXT_JUSTIFICATION_CENTER_BIT);
		addElement(servoList = new spehs::GUIRectangleScrollList(_batchManager));
	}

	ServoCreator::~ServoCreator()
	{

	}

	void ServoCreator::inputUpdate(InputUpdateData& data)
	{
		GUIRectangleColumn::inputUpdate(data);

		if (inputManager->isKeyPressed(MOUSEBUTTON_LEFT))
		{//Mouse button pressed

			if (createButton->getMouseHover())
				tryCreate();

			for (unsigned i = 0; i < servoList->elementsSize(); i++)
			{
				ServoCreator* servoCreator = (ServoCreator*)servoList->at(i);
				if (servoCreator->at(0/*remove*/)->getMouseHover())
				{
					servoList->removeElement(servoCreator);
				}
			}
		}
	}

	void ServoCreator::tryCreate()
	{
		const codex::gpio::Pin pin = codex::gpio::getPinNumberAsEnum(pinEditor->getEditorValue());
		if (pin == codex::gpio::Pin::pin_none)
		{
			codex::log::info("Cannot create a servo with invalid pin.");
			return;
		}

		const std::string name = nameEditor->getEditorValue();
		if (name.empty())
		{
			codex::log::info("Cannot create a servo without a name.");
			return;
		}

		for (size_t i = 0; i < servoList->elementsSize(); i++)
		{
			if (servoList->at(i)->getString() == name)
			{
				codex::log::info("A servo with the name '" + name + "' has already been created.");
				return;
			}
		}
		
		sync::Handle<codex::device::ServoGhost> servoHandle = syncManager.create<codex::device::ServoGhost>(name);
		if (servoHandle)
		{
			//Servo configuration setup
			servoHandle->setActive(false);
			servoHandle->setPin(pin);
			servoHandle->setMinAngle(codex::time::microseconds(minFrequencyEditor->getEditorValue()), minAngleEditor->getEditorValue());
			servoHandle->setMaxAngle(codex::time::microseconds(maxFrequencyEditor->getEditorValue()), maxAngleEditor->getEditorValue());
			servoHandle->setRotationSpeed(rotationSpeedEditor->getEditorValue());

			//Add element
			servoList->addElement(new ServoElement(batchManager, servoHandle));
		}
	}
}