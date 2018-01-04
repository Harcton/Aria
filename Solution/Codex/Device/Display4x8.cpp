#include "Display4x8.h"
#include "GPIO.h"
#include "SpehsEngine/Core/Time.h"


namespace codex
{
	namespace device
	{
		Display4x8::Display4x8()
		{
		}


		Display4x8::~Display4x8()
		{

		}

		void Display4x8::setString(const std::string& str)
		{
			const codex::gpio::Pin digitPins[4] =
			{
				codex::gpio::pin_19,
				codex::gpio::pin_23,
				codex::gpio::pin_21,
				codex::gpio::pin_24,
			};
			const codex::gpio::Pin ledPins[8] =
			{
				codex::gpio::pin_38,//top
				codex::gpio::pin_40,//top left
				codex::gpio::pin_36,//top right
				codex::gpio::pin_37,//middle
				codex::gpio::pin_29,//bottom left
				codex::gpio::pin_35,//bottom right
				codex::gpio::pin_31,//bottom
				codex::gpio::pin_33,//.
			};
			const int symbols[12][8] =
			{
				{ /*top*/1,/*topL*/1,/*topR*/1,/*middle*/0,/*bottomL*/1,/*bottomR*/1,/*bottom*/1,/*.*/0 },//0
				{ /*top*/0,/*topL*/0,/*topR*/1,/*middle*/0,/*bottomL*/0,/*bottomR*/1,/*bottom*/0,/*.*/0 },//1
				{ /*top*/1,/*topL*/0,/*topR*/1,/*middle*/1,/*bottomL*/1,/*bottomR*/0,/*bottom*/1,/*.*/0 },//2
				{ /*top*/1,/*topL*/0,/*topR*/1,/*middle*/1,/*bottomL*/0,/*bottomR*/1,/*bottom*/1,/*.*/0 },//3
				{ /*top*/0,/*topL*/1,/*topR*/1,/*middle*/1,/*bottomL*/0,/*bottomR*/1,/*bottom*/0,/*.*/0 },//4
				{ /*top*/1,/*topL*/1,/*topR*/0,/*middle*/1,/*bottomL*/0,/*bottomR*/1,/*bottom*/1,/*.*/0 },//5
				{ /*top*/1,/*topL*/1,/*topR*/0,/*middle*/1,/*bottomL*/1,/*bottomR*/1,/*bottom*/1,/*.*/0 },//6
				{ /*top*/1,/*topL*/0,/*topR*/1,/*middle*/0,/*bottomL*/0,/*bottomR*/1,/*bottom*/0,/*.*/0 },//7
				{ /*top*/1,/*topL*/1,/*topR*/1,/*middle*/1,/*bottomL*/1,/*bottomR*/1,/*bottom*/1,/*.*/0 },//8
				{ /*top*/1,/*topL*/1,/*topR*/1,/*middle*/1,/*bottomL*/0,/*bottomR*/1,/*bottom*/1,/*.*/0 },//9
				{ /*top*/0,/*topL*/0,/*topR*/0,/*middle*/0,/*bottomL*/0,/*bottomR*/0,/*bottom*/0,/*.*/1 },//.
				{ /*top*/0,/*topL*/0,/*topR*/0,/*middle*/0,/*bottomL*/0,/*bottomR*/0,/*bottom*/0,/*.*/0 },//<empty>
			};

			for (size_t i = 0; i < 4; i++)
				codex::gpio::setPinAsOutput(digitPins[i]);
			for (size_t i = 0; i < 8; i++)
				codex::gpio::setPinAsOutput(ledPins[i]);
			for (size_t i = 0; i < 4; i++)
				codex::gpio::enable(digitPins[i]);
			for (size_t i = 0; i < 8; i++)
				codex::gpio::disable(ledPins[i]);

			int number = 0;
			int digitIndex = 0;
			for (size_t d = 0; d < 4; d++)
			{//For each digit

				for (size_t d2 = 0; d2 < 4; d2++)
				{
					if (d == d2)
						codex::gpio::disable(digitPins[d2]);
					else
						codex::gpio::enable(digitPins[d2]);
				}

				for (size_t s = 0; s < 8; s++)
					codex::gpio::write(ledPins[s], (codex::gpio::PinState)symbols[number][s]);

				spehs::time::delay(spehs::time::fromMilliseconds(100));
				if (++number == 11)
				{
					number = 0;
					if (++digitIndex == 4)
						digitIndex = 0;
				}
			}
		}
	}
}