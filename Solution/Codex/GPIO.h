#pragma once
#include <bcm2835.h>
#include <stdint.h>
#include "SpehsEngine/Core/Time.h"
#undef delay //"delay" is a macro defined in the bcm header...



namespace codex
{
	namespace gpio
	{
		enum Pin : uint8_t
		{
			//PI version 2+
			pin_none = 0,
			pin_3 = RPI_V2_GPIO_P1_03,
			pin_5 = RPI_V2_GPIO_P1_05,
			pin_7 = RPI_V2_GPIO_P1_07,
			pin_8 = RPI_V2_GPIO_P1_08,
			pin_10 = RPI_V2_GPIO_P1_10,
			pin_11 = RPI_V2_GPIO_P1_11,
			pin_12 = RPI_V2_GPIO_P1_12,
			pin_13 = RPI_V2_GPIO_P1_13,
			pin_15 = RPI_V2_GPIO_P1_15,
			pin_16 = RPI_V2_GPIO_P1_16,
			pin_18 = RPI_V2_GPIO_P1_18,
			pin_19 = RPI_V2_GPIO_P1_19,
			pin_21 = RPI_V2_GPIO_P1_21,
			pin_22 = RPI_V2_GPIO_P1_22,
			pin_23 = RPI_V2_GPIO_P1_23,
			pin_24 = RPI_V2_GPIO_P1_24,
			pin_26 = RPI_V2_GPIO_P1_26,
			pin_29 = RPI_V2_GPIO_P1_29,
			pin_31 = RPI_V2_GPIO_P1_31,
			pin_32 = RPI_V2_GPIO_P1_32,
			pin_33 = RPI_V2_GPIO_P1_33,
			pin_35 = RPI_V2_GPIO_P1_35,
			pin_36 = RPI_V2_GPIO_P1_36,
			pin_37 = RPI_V2_GPIO_P1_37,
			pin_38 = RPI_V2_GPIO_P1_38,
			pin_40 = RPI_V2_GPIO_P1_40
		};
		enum PinState : uint8_t
		{
			low = LOW,
			high = HIGH,
			invalid
		};
		enum PinMode : uint8_t
		{
			input,
			output,
		};
		extern void enable(const Pin pin);
		extern void disable(const Pin pin);
		extern void write(const Pin pin, const PinState pinState);
		
		extern PinState read(const Pin pin);

		/*
			Measures time it takes for pin to go from !pinState to pinState and back to !pinState.
			Blocks indefinitely if no timeout parameter is specified (set to 0).
			In case of a timeout, returns 0.
		*/
		extern spehs::time::Time pulseIn(const Pin pin, const PinState pinState, const spehs::time::Time timeout = 0);

		extern void setPinMode(const Pin pin, const PinMode mode);
		extern void setPinAsInput(const Pin pin);
		extern void setPinAsOutput(const Pin pin);

		//Pin enumeration conversion: the enumerated values do not match the naming.
		/* Returns 0 if provided number is invalid. */
		extern Pin getPinNumberAsEnum(const unsigned number);
		extern unsigned getPinEnumAsNumber(const Pin pin);
	}
}