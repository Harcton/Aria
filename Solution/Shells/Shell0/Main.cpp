#include <Codex/Log.h>
#include <Codex/Codex.h>
#include "Shell0.h"


/*
	Arguments:
	0: path
	1: connect address
	2. connect port

	or

	0: path
	1. receive port
*/
int main(const int argc, const char** argv)
{
	codex::initialize(argc, argv);

	while (1)
	{
		Shell0 shell0;
		shell0.start(argc, argv);
	}

	codex::uninitialize();
	return 0;
}


/*
//TEST BASE

std::atomic<int> strength(0);
std::atomic<bool> keepRunningInput(false);
void runInput()
{
keepRunningInput = true;
while (keepRunningInput)
{
codex::log::info("Input strength:\n>");
int val;
std::cin >> val;
strength = val;
}
}

//Main
std::thread inputThread(runInput);
codex::gpio::Pin pin = codex::gpio::pin_11;
codex::gpio::setPinAsOutput(pin);
while (strength >= 0)
{
codex::time::TimeType interval = codex::time::milliseconds(1);
codex::time::delay(interval);
}
inputThread.join();

*/