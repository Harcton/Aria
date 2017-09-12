#include "GPIO.h"
#include "Log.h"


namespace codex
{
	namespace gpio
	{
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
		PinState read(const Pin pin)
		{
			return bcm2835_gpio_lev(pin) == HIGH ? PinState::high : PinState::low;
		}
		void setPinMode(const Pin pin, const PinMode mode)
		{
			if (mode == PinMode::output)
				bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
			else if (mode == PinMode::input)
				bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
			else
				codex::log::error("setPinMode() error. Invalid pin mode.");
		}
		void setPinAsInput(const Pin pin)
		{
			bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
		}
		void setPinAsOutput(const Pin pin)
		{
			bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
		}
	}
}