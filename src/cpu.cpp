#include <stdint.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "cpu.h" 

cpu::cpu()
	: RA(0), RB(0), RC(0), RD(0), PC(0), SP(0), BP(0), RF(0), CMPREG(0), clockHalted(false), cycleCount(0)
{
	this->memoryArray = new uint8_t[CPU_AVALIABLE_MEMORY]{0};
}

cpu::~cpu()
{
	delete[] this->memoryArray;
}

bool cpu::clockTick()
{
	if (clockHalted)
	{
		return false;
	}
	cycleCount++;
	instructionData data;
	incrementAndFetch(data);
	decodeAndExecute(data);

	return true;
}

bool cpu::loadBinaryImage(std::string path)
{
	std::ifstream binFile(path, std::ios::in | std::ios::binary);
	if (!binFile.is_open()) { return false; }
	binFile.read((char*)memoryArray, CPU_AVALIABLE_MEMORY);
	return true;
}

bool cpu::getHaltState()
{
	return clockHalted;
}



std::string cpu::instructionEnumToName(instructions instr)
{
	switch (instr) {
		case instructions::NOP:
			return "NOP";
		case instructions::CALL:
			return "CALL";
		case instructions::RET:
			return "RET";
		case instructions::POP:
			return "POP";
		case instructions::PUSH:
			return "PUSH";
		case instructions::READIMM4:
			return "READIMM4";
		case instructions::READIMM2:
			return "READIMM2";
		case instructions::READIMM1:
			return "READIMM1";
		case instructions::READPTR4:
			return "READPTR4";
		case instructions::READPTR2:
			return "READPTR2";
		case instructions::READPTR1:
			return "READPTR1";
		case instructions::WRITEIMM4:
			return "WRITEIMM4";
		case instructions::WRITEIMM2:
			return "WRITEIMM2";
		case instructions::WRITEIMM1:
			return "WRITEIMM1";
		case instructions::WRITEPTR4:
			return "WRITEPTR4";
		case instructions::WRITEPTR2:
			return "WRITEPTR2";
		case instructions::WRITEPTR1:
			return "WRITEPTR1";
		case instructions::ADD:
			return "ADD";
		case instructions::DIV:
			return "DIV";
		case instructions::MUL:
			return "MUL";
		case instructions::MOV:
			return "MOV";
		case instructions::CMP:
			return "CMP";
		case instructions::JMP:
			return "JMP";
		case instructions::JMPIMM:
			return "JMPIMM";
		case instructions::JMPIF:
			return "JMPCMP";
		case instructions::JMPREL:
			return "JMPREL";
		case instructions::JMPRELIF:
			return "JMPRELCMP";
		case instructions::OUT:
			return "OUT";
		case instructions::IN:
			return "IN";
		case instructions::HALT:
			return "HALT";
		default:
			return "INVALID";
	}
}

uint32_t cpu::popStack()
{
	SP-=4;
	if (BP-SP-3 >=0 && BP-SP-3 < CPU_AVALIABLE_MEMORY )
	{
		uint32_t value = (*((uint32_t*)(memoryArray+BP-SP-3))); // dirty memory poking to allow for misaligned uint32_t insertion
		return value;
	}
	else
	{
		cpuDebugCheck("Stack underflow");
		return 0;
	}
}

uint32_t cpu::getCycleCount()
{
	return cycleCount;
}

void cpu::pushStack(uint32_t value)
{
	if (BP-SP-3 >=0 && BP-SP-3 < CPU_AVALIABLE_MEMORY )
	{
		*((uint32_t*)(memoryArray+BP-SP-3)) = value; // dirty memory poking to allow for misaligned uint32_t insertion
		SP+=4;
	}
	else
	{
		cpuDebugCheck("Stack underflow");
	}
}

void cpu::cpuDebugCheck(std::string errorMessage)
{
	clockHalted = true;
	std::cout << "\nCPU debug check\n" << errorMessage;
}

uint32_t cpu::readGeneralRegister(uint32_t index)
{
	switch (index)
	{
	case 0:
		return RA;
	case 1:
		return RB;
	case 2:
		return RC;
	case 3:
		return RD;
	case 4:
		return CMPREG;
	case 5:
		return SP;
	case 6:
		return BP;
	case 7:
		return RF;
	default:
		std::stringstream errorMessage;
		errorMessage << "Register read: Illegal register value, index: " << index;
		cpuDebugCheck(errorMessage.str());
		return 0;
	}
}

void cpu::writeGeneralRegister(uint32_t index, uint32_t value)
{
	switch (index)
	{
	case 0:
		RA = value;
		break;
	case 1:
		RB = value;
		break;
	case 2:
		RC = value;
		break;
	case 3:
		RD = value;
		break;
	case 4:
		CMPREG = (uint8_t)value;
		break;
	case 5:
		SP = value;
		break;
	case 6:
		BP = value;
		break;
	default:
		//std::cout << "Illegal: " << index << " value: " << value << "\n";
		std::stringstream errorMessage;
		errorMessage << "Register write: Illegal register value, index: " << index << " value: " << value;
		cpuDebugCheck(errorMessage.str());
		return;
	}
}

void cpu::writeMemory4(uint32_t index, uint32_t value)
{
	if (index < CPU_AVALIABLE_MEMORY)
	{
		//dirty memory write tehehe 
		// dirty memory poking to allow for misaligned uint32_t insertion
		*((uint32_t*)(memoryArray + index)) = value;
	}
	else
	{
		cpuDebugCheck("Memory write out of bound");
	}
}

void cpu::writeMemory2(uint32_t index, uint16_t value)
{
	if (index < CPU_AVALIABLE_MEMORY)
	{
		//dirty memory write tehehe 
		// dirty memory poking to allow for misaligned uint32_t insertion
		*((uint16_t*)(memoryArray + index)) = value;
	}
	else
	{
		cpuDebugCheck("Memory write out of bound");
	}
}

void cpu::writeMemory1(uint32_t index, uint8_t value)
{
	if (index < CPU_AVALIABLE_MEMORY)
	{
		memoryArray[index] = value;
	}
	else
	{
		cpuDebugCheck("Memory write out of bound");
	}
}

void cpu::incrementAndFetch(instructionData& instructionObj)
{
	if (this->PC > CPU_AVALIABLE_MEMORY - 16)
	{
		cpuDebugCheck("PC overflow");
		return;
	}
	
	instructionObj.instr = (instructions)(*(uint32_t*)(this->memoryArray+PC));
	instructionObj.oprandA = (*(uint32_t*)(this->memoryArray+PC+4));
	instructionObj.oprandB = (*(uint32_t*)(this->memoryArray+PC+8));
	instructionObj.oprandC = (*(uint32_t*)(this->memoryArray+PC+12));
	PC += 16;
}

uint32_t cpu::flipEndian(uint32_t n)
{
	return (n << 24) | ((n<<8) & 0x00ff0000) | ((n >> 8) & 0x0000ff00) | ((n >> 8) & 0x0000ff00) | ((n >> 24) & 0x000000ff);
}

uint16_t cpu::flipEndian(uint16_t n)
{
	return (n << 8) | (n >> 8);
}

instructionData::instructionData()
	: instr((cpu::instructions)0), oprandA(0), oprandB(0), oprandC(0)
{
}

instructionData::instructionData(cpu::instructions _instr, uint32_t _oprandA, uint32_t _oprandB, uint32_t _oprandC)
	: instr(_instr), oprandA(_oprandA), oprandB(_oprandB), oprandC(_oprandC)
{}

void cpu::handleCMPInstruction(instructionData& instructionObj)
{
	bool result = false;
	switch (CMPREG)
	{
	case CMPenum::EQUAL:
		result = readGeneralRegister(instructionObj.oprandA) == readGeneralRegister(instructionObj.oprandB);
		break;
	case CMPenum::GREATER_THAN:
		result = readGeneralRegister(instructionObj.oprandA) > readGeneralRegister(instructionObj.oprandB);
		break;
	case CMPenum::GREATER_THAN_OR_EQUAL:
		result = readGeneralRegister(instructionObj.oprandA) >= readGeneralRegister(instructionObj.oprandB);
		break;
	case CMPenum::LESS_THAN:
		result = readGeneralRegister(instructionObj.oprandA) < readGeneralRegister(instructionObj.oprandB);
		break;
	case CMPenum::LESS_THAN_OR_EQUAL:
		result = readGeneralRegister(instructionObj.oprandA) <= readGeneralRegister(instructionObj.oprandB);
		break;
	case CMPenum::NOT_EQUAL:
		result = readGeneralRegister(instructionObj.oprandA) != readGeneralRegister(instructionObj.oprandB);
		break;
	default:
		cpuDebugCheck("Unknown cmp value");
	}

	RF = (RF & 0b11111110) | result;
}

void cpu::handleOutInstruction(instructionData& instructionObj)
{
	switch (instructionObj.oprandB)
	{
	case 0:
		//std::cout << (char)(readGeneralRegister(instructionObj.oprandA) & 0xFF);
		break;
	}
}

bool cpu::decodeAndExecute(instructionData& instructionObj)
{
	uint32_t address;
	uint64_t result;
	switch (instructionObj.instr)
	{
	case instructions::MOV:
		writeGeneralRegister(instructionObj.oprandA, instructionObj.oprandB);
		break;
	case instructions::WRITEIMM4:
		writeMemory4(instructionObj.oprandA, instructionObj.oprandB);
		break;
	case instructions::WRITEIMM2:
		writeMemory2(instructionObj.oprandA, instructionObj.oprandB);
		break;
	case instructions::WRITEIMM1:
		writeMemory1(instructionObj.oprandA, instructionObj.oprandB);
		break;
	case instructions::INC:
		result = addAndSetFlags((uint64_t)readGeneralRegister(instructionObj.oprandA), 1);
		writeGeneralRegister(instructionObj.oprandA, (uint32_t)result);
		break;
	case instructions::DEC:
		result = subtractAndSetFlags((uint64_t)readGeneralRegister(instructionObj.oprandA), 1);
		writeGeneralRegister(instructionObj.oprandA, (uint32_t)result);
		break;
	case instructions::PUSHREG:
		pushStack(RA);
		pushStack(RB);
		pushStack(RC);
		pushStack(RD);
		pushStack(CMPREG);
		break;
	case instructions::POPREG:
		CMPREG = popStack();
		RD = popStack();
		RC = popStack();
		RB = popStack();
		RA = popStack();
		break;
	case instructions::RET:
		PC = popStack();
		break;
	case instructions::CALL:
		pushStack(PC);
		PC = instructionObj.oprandA;
		break;
	case instructions::PUSH:
		pushStack(readGeneralRegister(instructionObj.oprandA));
		break;
	case instructions::POP:
		writeGeneralRegister(instructionObj.oprandA, popStack());
		break;
	case instructions::JMPIF:
		address = readGeneralRegister(instructionObj.oprandA);
		if (address < CPU_AVALIABLE_MEMORY && (RF&0x1))
		{
			PC = address;
		}
		break;
	case instructions::CMP:
		handleCMPInstruction(instructionObj);
		break;
	case instructions::NOP:
		break;
	case instructions::JMP:
		address = readGeneralRegister(instructionObj.oprandA);
		if (address < CPU_AVALIABLE_MEMORY)
		{
			PC = address;
		}
		break;
	case instructions::HALT:
		clockHalted = true;
		break;
	case instructions::OUT:
		handleOutInstruction(instructionObj);
		break;
	case instructions::READPTR1: // arg1: Reg Index PTR / arg2: Reg Index Ouput
		writeGeneralRegister(instructionObj.oprandA, readMemory1(readGeneralRegister(instructionObj.oprandB)));
		break;
	case instructions::SUB:
		writeGeneralRegister(instructionObj.oprandA, subtractAndSetFlags(instructionObj.oprandA, readGeneralRegister(instructionObj.oprandB)));
		break;
	case instructions::ADD:
		result = (uint64_t)readGeneralRegister(instructionObj.oprandA) + (uint64_t)readGeneralRegister(instructionObj.oprandB);
		writeGeneralRegister(instructionObj.oprandA, (uint32_t)result);
		RF = (RF & 0b11111101) | ((uint32_t)result & 2147483648);
		RF = (RF & 0b11111011) | ((result >> 32)!=0);
		break;
	case instructions::XOR:
		writeGeneralRegister(instructionObj.oprandA, readGeneralRegister(instructionObj.oprandA) ^ readGeneralRegister(instructionObj.oprandB));
		break;
	case instructions::AND:
		writeGeneralRegister(instructionObj.oprandA, readGeneralRegister(instructionObj.oprandA) & readGeneralRegister(instructionObj.oprandB));
		break;
	case instructions::OR:
		writeGeneralRegister(instructionObj.oprandA, readGeneralRegister(instructionObj.oprandA) | readGeneralRegister(instructionObj.oprandB));
		break;
	case instructions::JMPIMM:
		if (instructionObj.oprandA < CPU_AVALIABLE_MEMORY)
		{
			PC = instructionObj.oprandA;
		}
		break;
	default:
		std::stringstream errorStream;
		errorStream << "Unknown instruction: " << instructionObj.instr;
		cpuDebugCheck(errorStream.str());
		break;
	}
	return true;
}

int64_t cpu::addAndSetFlags(int64_t a, int64_t b)
{
    int64_t sum = a+b;
	if ((uint64_t)(sum>>32))
	{
		this->RF|=0b00000100;
	}
	else
	{
		this->RF&=0b11111011;
	}

	if (a > 0 && b > UINT32_MAX - b)
	{
		this->RF|=0b00001000;
	}
	else
	{
		this->RF&=0b11110111;
	}
	return sum;
}

int64_t cpu::subtractAndSetFlags(int64_t a, int64_t b)
{
    int64_t sum = a-b;
	if ((uint64_t)(sum>>32))
	{
		this->RF|=0b00000100;
	}
	else
	{
		this->RF&=0b11111011;
	}

	if (b < 0 && a > INT_MAX + b)
	{
		this->RF|=0b00001000;
	}
	else
	{
		this->RF&=0b11110111;
	}
	return sum;
}

uint8_t cpu::readMemory1(uint32_t address)
{
	if (address < CPU_AVALIABLE_MEMORY)
	{
		return memoryArray[address];
	}
	cpuDebugCheck("Memory read out of bound");
	return 0;
}
