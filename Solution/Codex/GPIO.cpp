#include "GPIO.h"


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
	}
}