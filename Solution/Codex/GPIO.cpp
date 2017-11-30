#include "Codex/GPIO.h"
#include "Codex/CodexAssert.h"
#include "Codex/Log.h"


namespace codex
{
	namespace gpio
	{
#ifdef CODEX_GPIO
		void enable(const Pin pin)
		{
			bcm2835_gpio_write(pin, HIGH);
		}
		void disable(const Pin pin)
		{
			bcm2835_gpio_write(pin, LOW);
		}
		void write(const Pin pin, const PinState pinState)
		{
			bcm2835_gpio_write(pin, pinState);
		}
		PinState read(const Pin pin)
		{
			return bcm2835_gpio_lev(pin) == HIGH ? PinState::high : PinState::low;
		}
		time::TimeType pulseIn(const Pin pin, const PinState pinState, const time::TimeType timeout)
		{
			bool readyToReceivePulse = false;
			bool pulseReceived = false;
			const time::TimeType beginTime = time::now();
			while (true)
			{
				if (gpio::read(pin) == pinState)
				{
					if (readyToReceivePulse)
						pulseReceived = true;
				}
				else
				{
					if (pulseReceived)
						break;
					else
						readyToReceivePulse = true;
				}
				if (timeout && time::now() - beginTime >= timeout)
					return 0;
			}
			return time::now() - beginTime;
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
#else//CODEX_GPIO is not defined, these functions are non-functional
		void enable(const Pin pin)
		{
			CODEX_ASSERT(false);
		}
		void disable(const Pin pin)
		{
			CODEX_ASSERT(false);
		}
		void write(const Pin pin, const PinState pinState)
		{
			CODEX_ASSERT(false);
		}
		PinState read(const Pin pin)
		{
			CODEX_ASSERT(false);
			return PinState::invalid;
		}
		time::TimeType pulseIn(const Pin pin, const PinState pinState, const time::TimeType timeout)
		{
			CODEX_ASSERT(false);
			return 0;
		}
		void setPinMode(const Pin pin, const PinMode mode)
		{
			CODEX_ASSERT(false);
		}
		void setPinAsInput(const Pin pin)
		{
			CODEX_ASSERT(false);
		}
		void setPinAsOutput(const Pin pin)
		{
			CODEX_ASSERT(false);
		}
#endif
	}
}