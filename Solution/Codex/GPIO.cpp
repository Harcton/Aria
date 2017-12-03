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
		Pin getPinNumberAsEnum(const unsigned number)
		{
			switch (number)
			{
			default: return pin_none;
			case 3: return pin_3;
			case 5: return pin_5;
			case 7: return pin_7;
			case 8: return pin_8;
			case 10: return pin_10;
			case 11: return pin_11;
			case 12: return pin_12;
			case 13: return pin_13;
			case 15: return pin_15;
			case 16: return pin_16;
			case 18: return pin_18;
			case 19: return pin_19;
			case 21: return pin_21;
			case 22: return pin_22;
			case 23: return pin_23;
			case 24: return pin_24;
			case 26: return pin_26;
			case 29: return pin_29;
			case 31: return pin_31;
			case 32: return pin_32;
			case 33: return pin_33;
			case 35: return pin_35;
			case 36: return pin_36;
			case 37: return pin_37;
			case 38: return pin_38;
			case 40: return pin_40;
			}
		}

		unsigned getPinEnumAsNumber(const Pin pin)
		{
			switch (pin)
			{
			default: return 0;
			case pin_3:  return 3;
			case pin_5:  return 5;
			case pin_7:  return 7;
			case pin_8:  return 8;
			case pin_10: return 10;
			case pin_11: return 11;
			case pin_12: return 12;
			case pin_13: return 13;
			case pin_15: return 15;
			case pin_16: return 16;
			case pin_18: return 18;
			case pin_19: return 19;
			case pin_21: return 21;
			case pin_22: return 22;
			case pin_23: return 23;
			case pin_24: return 24;
			case pin_26: return 26;
			case pin_29: return 29;
			case pin_31: return 31;
			case pin_32: return 32;
			case pin_33: return 33;
			case pin_35: return 35;
			case pin_36: return 36;
			case pin_37: return 37;
			case pin_38: return 38;
			case pin_40: return 40;
			}
		}

	}
}