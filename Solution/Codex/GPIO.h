#pragma once
#include <bcm2835.h>

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
			high = HIGH
		};
		void write(const Pin pin, const PinState pinState)
		{
			bcm2835_gpio_write(pin, pinState);
		}
		void enable(const Pin pin)
		{
			bcm2835_gpio_write(pin, HIGH);
		}
		void disable(const Pin pin)
		{
			bcm2835_gpio_write(pin, LOW);
		}
	}
}