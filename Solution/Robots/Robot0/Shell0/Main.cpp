#include <iostream>
#include <functional>
#include <atomic>
#include <Codex/Log.h>
#include <Codex/GPIO.h>
#include <Codex/Servo.h>
#include <Codex/Time.h>
#include <Codex/Codex.h>
#include <Codex/Protocol.h>
#include <Codex/SocketTCP.h>
#include <bcm2835.h>

#define PIN RPI_GPIO_P1_12
// and it is controlled by PWM channel 0
#define PWM_CHANNEL 0
// This controls the max range of the PWM signal
#define RANGE 1024


std::atomic<int> receivedHandlerCallCount(0);
void receiveHandler(const void* data, const size_t bytes)
{
	codex::log::info("Receive handler received " + std::to_string(bytes) + " bytes");
	receivedHandlerCallCount++;
}


std::atomic<int> analogSignal(0);
void inputThreadRun()
{
	while (analogSignal >= 0 && analogSignal < RANGE)
	{
		codex::log::info("Set analog signal:");
		int _analogSignal;
		std::cin >> _analogSignal;
		analogSignal = _analogSignal;
	}
}



int main(int argc, char** argv)
{
	codex::initialize();
	//codex::log::info("Shell0 initializing... 4");


	//codex::log::info("Motor test 1");
	//Set pins as output
	codex::log::info("this out");
	//codex::gpio::Pin aPulseWidth = codex::gpio::pin_12;
	codex::gpio::Pin aPulseWidth = codex::gpio::pin_5;
	codex::gpio::Pin aInput1 = codex::gpio::pin_13;
	codex::gpio::Pin aInput2 = codex::gpio::pin_11;
	codex::gpio::Pin bInput1 = codex::gpio::pin_15;
	codex::gpio::Pin bInput2 = codex::gpio::pin_16;
	codex::gpio::Pin bPulseWidth = codex::gpio::pin_7;
	bcm2835_gpio_fsel(aPulseWidth, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(aInput1, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(aInput2, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(bInput1, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(bInput2, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(bPulseWidth, BCM2835_GPIO_FSEL_OUTP);
	//

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



	// Set the output pin to Alt Fun 5, to allow PWM channel 0 to be output there
	bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_ALT5);
	// Clock divider is set to 16.
	// With a divider of 16 and a RANGE of 1024, in MARKSPACE mode,
	// the pulse repetition frequency will be
	// 1.2MHz/1024 = 1171.875Hz, suitable for driving a DC motor with PWM
	bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_16);
	bcm2835_pwm_set_mode(PWM_CHANNEL, 1, 1);
	bcm2835_pwm_set_range(PWM_CHANNEL, RANGE);

	/*
	int direction = 1; // 1 is increase, -1 is decrease
	int data = 1;
	while (1)
	{
		if (data == 1)
			direction = 1;   // Switch to increasing
		else if (data == RANGE - 1)
			direction = -1;  // Switch to decreasing
		data += direction;
		bcm2835_pwm_set_data(PWM_CHANNEL, data);
		codex::gpio::disable(aInput1);
		codex::gpio::enable(aInput2);
		codex::time::delay(codex::time::nanoseconds(500000));
	}
	*/

	bcm2835_pwm_set_data(PWM_CHANNEL, RANGE - 1);
	codex::gpio::disable(aInput1);
	codex::gpio::enable(aInput2);
	bcm2835_delay(1000);
	codex::gpio::enable(aInput1);
	codex::gpio::disable(aInput2);
	bcm2835_delay(1000);
	codex::gpio::disable(aInput1);
	codex::gpio::disable(aInput2);
	bcm2835_delay(4000);



	std::thread inputThread(inputThreadRun);
	int prevAnalogSignal = -1;
	while (analogSignal >= 0 && analogSignal < RANGE)
	{
		if (analogSignal != prevAnalogSignal)
		{
			codex::gpio::disable(aInput1);
			codex::gpio::enable(aInput2);

			codex::time::delay(codex::time::milliseconds(500));

			codex::gpio::enable(aInput1);
			codex::gpio::disable(aInput2);
			bcm2835_pwm_set_data(PWM_CHANNEL, analogSignal);
			prevAnalogSignal = analogSignal;
			codex::log::info("Analog signal set to " + std::to_string(analogSignal));
		}
	}
	inputThread.join();
	bcm2835_pwm_set_data(PWM_CHANNEL, 0);

	/*
	//Create servo
	codex::Servo servo;
	servo.setPin(codex::gpio::pin_3);
	servo.mapPosition(codex::time::milliseconds(0), codex::time::milliseconds(2));
	servo.run();

	bool run = true;
	while (run)
	{
		int position;
		codex::log::info("Input position\n>");
		std::cin >> position;
		servo.setPosition((unsigned char)position);

		//Stop
		if (position < 0)
		{
			run = false;
			servo.stop();
		}
	}
	*/


	/*
	uint64_t someData;
	codex::protocol::WriteBuffer writeBuffer;
	writeBuffer.resize(sizeof(someData));
	writeBuffer.write(&someData, sizeof(someData));

	codex::SocketTCP socketTCP;
	socketTCP.resizeReceiveBuffer(64000);
	if (socketTCP.connect("192.168.10.52", 41523))
	codex::log::info("Successfully connected the tcp socket!");
	if (socketTCP.startReceiving(std::bind(&receiveHandler, std::placeholders::_1, std::placeholders::_2)))
	codex::log::info("TCP socket has began successfully receiving data!");
	if (socketTCP.sendPacket(writeBuffer) == writeBuffer.getSize())
	codex::log::info("TCP socket successfully sent a packet!");

	codex::log::info("Awaiting for a response...");
	while (receivedHandlerCallCount == 0)
	{
	//Blocks
	}
	std::getchar();
	*/


	codex::uninitialize();
	return 0;
}