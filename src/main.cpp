#include <iostream>
#include "cpu.h"
#include <chrono>
int main()
{
	cpu CPU;
	CPU.loadBinaryImage("program.bin");
	auto start = std::chrono::high_resolution_clock::now();
	while (!CPU.getHaltState())
	{
		CPU.clockTick();
	}
	double executionTime = (double)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() / 1000000.0;
	std::cout << executionTime <<"\n";
	std::cout << CPU.getCycleCount()<< " cycles " << CPU.getCycleCount()/executionTime/1000000 << "mhz" << "\n";

	return 0;
}