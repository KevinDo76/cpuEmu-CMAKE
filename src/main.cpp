#include <iostream>
#include "cpu.h"
#include <chrono>
int main()
{
	cpu CPU;
	CPU.loadBinaryImage("program.bin");
	while (!CPU.getHaltState())
	{
		CPU.clockTick();
	}
	return 0;
}