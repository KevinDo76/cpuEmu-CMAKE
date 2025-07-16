#pragma once
#define CPU_AVALIABLE_MEMORY 0x20000

#include <stdint.h>
#include <limits.h>
#include <mutex>
#include <thread>

class instructionData;

class cpu
{
public:
	cpu();
	~cpu();
	cpu(const cpu&) = delete;
	cpu& operator=(const cpu&) = delete;
	bool clockTick();
	bool loadBinaryImage(std::string);
	bool getHaltState();

	enum CMPenum {
		EQUAL = 0,
		NOT_EQUAL = 1,
		LESS_THAN = 2,
		GREATER_THAN = 3,
		LESS_THAN_OR_EQUAL = 4,
		GREATER_THAN_OR_EQUAL = 5,
	};

	enum instructions {
/*Done*/NOP        = 0x00,    // no oprand
/*Done*/CALL       = 0x01,    // arg1: Address<32>, push PC to Stack
/*Done*/RET        = 0x02,    // no oprand, pull PC from Stack
/*Done*/POP        = 0x03,	  // arg1: Reg Index Result
/*Done*/PUSH       = 0x04,	  // arg1: Reg Index Value
		READIMM4   = 0x05,	  // arg1: Reg Index Result / arg2: Address<32>
		READIMM2   = 0x06,    // arg1: Reg Index Result / arg2: Address<32>
		READIMM1   = 0x07,    // arg1: Reg Index Result / arg2: Address<32>
		READPTR4   = 0x08,    // arg1: Reg Index Result / arg2: Reg Index PTR 
		READPTR2   = 0x09,    // arg1: Reg Index Result / arg2: Reg Index PTR 
/*Done*/READPTR1   = 0x0a,    // arg1: Reg Index Result / arg2: Reg Index PTR 
/*Done*/WRITEIMM4  = 0x0b,    // arg1: Address<32> / arg2: Reg Index Value
/*Done*/WRITEIMM2  = 0x0c,	  // arg1: Address<32> / arg2: Reg Index Value
/*Done*/WRITEIMM1  = 0x0d,	  // arg1: Address<32> / arg2: Reg Index Value
		WRITEPTR4  = 0x0e,	  // arg1: Reg Index PTR / arg2: Reg Index Value
		WRITEPTR2  = 0x0f,	  // arg1: Reg Index PTR / arg2: Reg Index Value
		WRITEPTR1  = 0x10,	  // arg1: Reg Index PTR / arg2: Reg Index Value
/*Done*/ADD        = 0x11,	  // arg1: Reg Index Result/Oprand A / arg2: Reg Index Oprand B
		DIV        = 0x12,	  // arg1: Reg Index Result/Oprand A / arg2: Reg Index Oprand B / arg3: Reg Index Result Remainder
		MUL        = 0x13,	  // arg1: Reg Index Result/Oprand A / arg2: Reg Index Oprand B
		LSHIFT	   = 0x14,    // arg1: Reg Index Result/Oprand A / arg2: Reg Index Amount
		RSHIFT     = 0x15,	  // arg1: Reg Index Result/Oprand A / arg2: Reg Index Amount
/*Done*/XOR        = 0x16,    // arg1: Reg Index Result/Oprand A / arg2: Reg Index Oprand B
/*Done*/AND        = 0x17,    // arg1: Reg Index Result/Oprand A / arg2: Reg Index Oprand B
/*Done*/OR         = 0x18,    // arg1: Reg Index Result/Oprand A / arg2: Reg Index Oprand B
/*Done*/MOV        = 0x19,	  // arg1: Reg Index Result / arg2: Value<32> 
/*Done*/CMP        = 0x1a,	  // arg1: Reg Index Value 1 / arg2: Reg Index Value 2
/*Done*/JMP        = 0x1b,	  // arg1: Reg Index Value
/*Done*/JMPIMM     = 0x1c,	  // arg1: Value<32>
/*Done*/JMPIF      = 0x1d,	  // arg1: Reg Index Jump Value(if cmp bit in RF is set)
		JMPREL     = 0x1e,	  // arg1: Reg Index Value<signed>
		JMPRELIF   = 0x1f,	  // arg1: Reg Index Check(jump if register is not zero) / arg2: Reg Index Jump Value<signed>
/*Done*/OUT        = 0x20,    // arg1: Reg Index Value / arg2: Address<32>
		IN         = 0x21,    // arg1: Reg Index Result / arg2: Address<32>
		INT        = 0x22,	  // no oprands, take code from reg RA;
/*Done*/HALT       = 0x23,	  // no oprand
/*Done*/INC        = 0x24,    // arg1: Reg Index Result 
/*Done*/DEC        = 0x25,    // arg1: Reg Index Result
/*Done*/PUSHREG    = 0x26,    // no oprands, push all register
/*Done*/POPREG     = 0x27,    // no oprands, pop all register
/*Done*/SUB        = 0x28,    // arg1: Reg Index Result/Oprand A / arg2: Reg Index Oprand B
		IDIV       = 0x29,    // arg1: Reg Index Result/Oprand A / args: Reg Index Oprand B
		IMUL       = 0x30,    // arg1: Reg Index Result/Oprand A / args: Reg Index Oprand B
/*Done*/CLHI       = 0x31,    // no oprand, disable hardware interrupt
/*Done*/STHI       = 0x32,    // no oprand, enable hardware interrupt
/*Done*/HIRET      = 0x33,    // no oprand, return from hardware interrupt
};

	static std::string instructionEnumToName(instructions instr);

	static uint32_t flipEndian(uint32_t n);
	static uint16_t flipEndian(uint16_t n);

	uint32_t popStack();
	uint64_t getCycleCount();
	void pushStack(uint32_t value);

	bool hardwareInterruptTriggered;
private:

	std::thread* outputThread;
	std::mutex outputMutex;
	std::string stringBuffer;
	bool isThreadRunning;
	void cpuDebugCheck(std::string errorMessage);

	bool clockHalted;

	uint32_t readGeneralRegister(uint32_t index);
	void writeGeneralRegister(uint32_t index, uint32_t value);

	void writeMemory4(uint32_t index, uint32_t value);
	void writeMemory2(uint32_t index, uint16_t value);
	void writeMemory1(uint32_t index, uint8_t value);

	void incrementAndFetch(instructionData& instructionObj);

	void handleCMPInstruction(instructionData& instructionObj);
	void handleOutInstruction(instructionData& instructionObj);

	bool decodeAndExecute(instructionData& instructionObj);
	//internal operation
	int64_t addAndSetFlags(int64_t a, int64_t b);
	int64_t subtractAndSetFlags(int64_t a, int64_t b);
	uint64_t unsignedMulAndSetFlags(uint64_t a, uint64_t b);
	uint64_t unsignedDivAndSetFlags(uint64_t a, uint64_t b, uint64_t& remainder);
	uint64_t cycleCount;

	uint8_t readMemory1(uint32_t address);

	uint32_t RA; // General Purpose Register
	uint32_t RB; // General Purpose Register
	uint32_t RC; // General Purpose Register
	uint32_t RD; // General Purpose Register
	uint32_t PC; // Program Counter
	uint32_t SP; // Stack pointer
	uint32_t BP; // Stack Base

	uint32_t HIREG; //hardware interrupt pointer
	uint8_t CMPREG; //ALU compare code.
	uint8_t RF; //flag register 
	// bit layout
	/*
	-- most significant
	0
	0
	0
	0 - Hardware Interrupt Enabled
	
	0 - Overflow/Underflow Flag (incorrect result in unsigned operation)
	0 - Carry Flag (incorrect result in unsigned operation)
	0 - Sign Flag
	0 - Compare success
	-- least significant
	
	*/

	uint8_t* memoryArray;

	bool isInHardwareInterrupt;
};


class instructionData
{
public:
	cpu::instructions instr;
	uint32_t oprandA;
	uint32_t oprandB;
	uint32_t oprandC;

	instructionData();
	instructionData(cpu::instructions _instr, uint32_t _oprandA, uint32_t _oprandB, uint32_t _oprandC);
};
