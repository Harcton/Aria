#include <Codex/Log.h>
#include <bcm2835.h>
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <atomic>

const uint64_t ms = 1000000;
int64_t ns = ms * 2.0;
std::atomic<bool> run(true);

void nsDelay(const uint64_t nanoSeconds)
{
	timespec t1; t1.tv_sec = 0; t1.tv_nsec = nanoSeconds;
	timespec t2;
	nanosleep(&t1, &t2);
}
void msDelay(const uint64_t microSeconds)
{
	bcm2835_delay(microSeconds);
}

void inputThread()
{
	while (run)
	{
		std::cout << "\nInput ms * 0.1: ";
		std::cin >> ns;
		ns *= ms / 10;
		if (ns < 0)
			run = false;
	}
}

int main(int argc, char** argv)
{
	boost::asio::io_service ioService;
	boost::asio::ip::tcp::socket socketTCP(ioService);
	std::cout << "\nShell0 initializing...";

	codex::log::info((const unsigned char*)"Codex test4");

	//BCM2835 blink test
	if (!bcm2835_init())
		return 1;
	// Set the pin to be an output
	auto pin = RPI_V2_GPIO_P1_03;
	bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
	// Blink

	std::thread input(&inputThread);

	while (run)
	{
		static uint64_t prevNs;
		bcm2835_gpio_write(pin, HIGH);
		nsDelay(prevNs = ns);
		bcm2835_gpio_write(pin, LOW);
		msDelay(20);
		//while (ns == prevNs)
		//{/*Blocks*/}

		//msDelay(500);
		//ns += ms / 10;
		//if (ns > 2.5 * ms)
		//{
		//	ns = 0;
		//	msDelay(2000);
		//	bcm2835_gpio_write(pin, HIGH);
		//	nsDelay(ns);
		//	bcm2835_gpio_write(pin, LOW);
		//	msDelay(2000);
		//}

		/*
		// Turn it on
		bcm2835_gpio_write(pin, HIGH);
		std::cout << "\nPin on";

		// wait a bit
		bcm2835_delay(500);

		// turn it off
		bcm2835_gpio_write(pin, LOW);
		std::cout << "\nPin off";

		// wait a bit
		bcm2835_delay(500);
		*/
	}
	bcm2835_close();

	input.join();

	return 0;
}