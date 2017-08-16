#include "DCMotor.h"
#include "GPIO.h"
#include "CodexTime.h"
#include <bcm2835.h>


namespace codex
{
	DCMotor::DCMotor()
	{
		//Just some example code to get things started...
		codex::gpio::Pin aPulseWidth = codex::gpio::pin_5;
		codex::gpio::Pin aInput1 = codex::gpio::pin_13;
		codex::gpio::Pin aInput2 = codex::gpio::pin_11;
		codex::gpio::disable(aInput1);
		codex::gpio::enable(aInput2);
		const codex::time::TimeType pulseInterval = codex::time::nanoseconds(2000000);
		int direction = 1;
		codex::time::TimeType data = 0;
		while (1)
		{
			if (data <= 0)
				direction = 1000;
			else if (data >= pulseInterval)
				direction = -1000;
			data += direction;
			codex::gpio::enable(aPulseWidth);
			codex::time::delay(data);
			codex::gpio::disable(aPulseWidth);
			codex::time::delay(pulseInterval - data);
		}
	}
	
	DCMotor::~DCMotor()
	{
	}
}