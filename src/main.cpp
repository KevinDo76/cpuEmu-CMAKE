#include <iostream>
#include "cpu.h"
#include <chrono>
#include <thread>
int main()
{
	cpu CPU;
	bool isFullSpeed = false;
	int clockSpeedHertz = 1000000;
	double divider1 = 100;
	double divider2 = 110;
	if (clockSpeedHertz<10000)
	{
		divider1 = 1;
		divider2 = 1;
	}
	int64_t timeNS = 1.0/((double)clockSpeedHertz/divider1) * 1000000000.0;
	
	CPU.loadBinaryImage("/home/coldest/Documents/Programming stuff/c++ stuff/CpuEmu-Assembler/program.bin");
	auto start = std::chrono::high_resolution_clock::now();
	auto startClockTime = std::chrono::high_resolution_clock::now();
	if (isFullSpeed)
	{
		while (!CPU.getHaltState())
		{
			if (CPU.getCycleCount()==200)
			{
				CPU.hardwareInterruptTriggered = true;
			}
			CPU.clockTick();
		}
	} else {
		while (!CPU.getHaltState())
		{
			startClockTime = std::chrono::high_resolution_clock::now();
			for (int i=0;i<divider2;i++)
			{
				CPU.clockTick();
				if (CPU.getCycleCount()==10000)
				{
					CPU.hardwareInterruptTriggered = true;
				}
			}
			while (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - startClockTime).count() < timeNS) {}
		}
	}
	double executionTime = (double)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() / 1000000.0;
	std::cout << "Execution Time: "<<executionTime <<"s\n";
	std::cout << CPU.getCycleCount()<< " cycles " << CPU.getCycleCount()/executionTime/1000000 << "mhz" << "\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	return 0;
}