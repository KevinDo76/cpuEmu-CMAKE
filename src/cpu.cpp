#include <stdint.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "cpu.h" 
#include <chrono>
#include <memory.h>

void handleOutput(bool* isRunning, std::string* buffer, std::mutex* mutex)
{
	//std::cout<<"Thread Started\n";
	while (*(isRunning))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		mutex->lock();
		//std::cout<<*buffer;
		*buffer = "";
		mutex->unlock();
	}
	//std::cout<<"Thread Exiting\n";
}

cpu::cpu()
	: RA(0), RB(0), RC(0), RD(0), PC(0), SP(0), BP(0), RF(0), CMPREG(0), HIREG(0), clockHalted(false), cycleCount(0), isInHardwareInterrupt(0), isThreadRunning(true)
{
	this->memoryArray = new uint8_t[CPU_AVALIABLE_MEMORY]{0};
	outputThread = new std::thread(&handleOutput, &isThreadRunning, &stringBuffer, &outputMutex);
}

cpu::~cpu()
{
	delete[] this->memoryArray;
	isThreadRunning = false;
	outputThread->join();
	delete outputThread;
}

bool cpu::clockTick()
{
	if (clockHalted)
	{
		isThreadRunning = false;
		return false;
	}
	cycleCount++;
	instructionData data;

	if ((RF & 0x10) && hardwareInterruptTriggered && !isInHardwareInterrupt)
	{
		isInHardwareInterrupt = true;
		RF &= 0xef;
		pushStack(PC);
		PC = HIREG;
	} else {
		incrementAndFetch(data);
	}
	hardwareInterruptTriggered = false;
	//std::cout<< instructionEnumToName(data.instr)<<"\n";
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

void cpu::loadBinaryArray(std::vector<char>& source)
{
	memcpy((void*)memoryArray, &source[0], source.size());
}

bool cpu::getHaltState()
{
	return clockHalted;
}



std::string cpu::instructionEnumToName(instructions instr)
{
	switch (instr) {
        case NOP:        return "NOP";
        case CALL:       return "CALL";
        case RET:        return "RET";
        case POP:        return "POP";
        case PUSH:       return "PUSH";
        case READIMM4:   return "READIMM4";
        case READIMM2:   return "READIMM2";
        case READIMM1:   return "READIMM1";
        case READPTR4:   return "READPTR4";
        case READPTR2:   return "READPTR2";
        case READPTR1:   return "READPTR1";
        case WRITEIMM4:  return "WRITEIMM4";
        case WRITEIMM2:  return "WRITEIMM2";
        case WRITEIMM1:  return "WRITEIMM1";
        case WRITEPTR4:  return "WRITEPTR4";
        case WRITEPTR2:  return "WRITEPTR2";
        case WRITEPTR1:  return "WRITEPTR1";
        case ADD:        return "ADD";
        case DIV:        return "DIV";
        case MUL:        return "MUL";
        case LSHIFT:     return "LSHIFT";
        case RSHIFT:     return "RSHIFT";
        case XOR:        return "XOR";
        case AND:        return "AND";
        case OR:         return "OR";
        case MOV:        return "MOV";
        case CMP:        return "CMP";
        case JMP:        return "JMP";
        case JMPIMM:     return "JMPIMM";
        case JMPIF:      return "JMPIF";
        case JMPREL:     return "JMPREL";
        case JMPRELIF:   return "JMPRELIF";
        case OUT:        return "OUT";
        case IN:         return "IN";
        case INT:        return "INT";
        case HALT:       return "HALT";
        case INC:        return "INC";
        case DEC:        return "DEC";
        case PUSHREG:    return "PUSHREG";
        case POPREG:     return "POPREG";
        case SUB:        return "SUB";
        case IDIV:       return "IDIV";
        case IMUL:       return "IMUL";
        case CLHI:       return "CLHI";
        case STHI:       return "STHI";
        case HIRET:      return "HIRET";
        default:         return "UNKNOWN";
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

uint64_t cpu::getCycleCount()
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
	case 8:
		return HIREG;
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
	case 8:
		HIREG = value;
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
		// dirty memory write tehehe 
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
		outputMutex.lock();
		stringBuffer += (char)(readGeneralRegister(instructionObj.oprandA) & 0xFF);
		outputMutex.unlock(); 
		//std::cout <<(char)(readGeneralRegister(instructionObj.oprandA) & 0xFF);
		break;
	}
}

bool cpu::decodeAndExecute(instructionData& instructionObj)
{
	uint32_t address;
	uint64_t result;
	uint64_t remainder=0;

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
		writeGeneralRegister(instructionObj.oprandA, subtractAndSetFlags(readGeneralRegister(instructionObj.oprandA), readGeneralRegister(instructionObj.oprandB)));
		break;
	case instructions::ADD:
		writeGeneralRegister(instructionObj.oprandA, addAndSetFlags(readGeneralRegister(instructionObj.oprandA), readGeneralRegister(instructionObj.oprandB)));
		break;
	case instructions::DIV:
		writeGeneralRegister(instructionObj.oprandA, unsignedDivAndSetFlags(readGeneralRegister(instructionObj.oprandA), readGeneralRegister(instructionObj.oprandB), remainder));
		writeGeneralRegister(instructionObj.oprandC, remainder);
		break;
	case instructions::MUL:
		writeGeneralRegister(instructionObj.oprandA, unsignedMulAndSetFlags(readGeneralRegister(instructionObj.oprandA), readGeneralRegister(instructionObj.oprandB)));
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
	case instructions::CLHI:
		RF &= 0xef;
		break;
	case instructions::STHI:
		if (!isInHardwareInterrupt)
		{
			RF |= 0x10;
		}
		break;
	case instructions::HIRET:
		if (!isInHardwareInterrupt)
		{
			cpuDebugCheck("Hardware interrupt return while not in interrupt");
		}
		RF |= 0x10;
		isInHardwareInterrupt = false;
		PC = popStack();
		break;
	default:
		std::stringstream errorStream;
		errorStream << "Unknown instruction: " << instructionObj.instr;
		cpuDebugCheck(errorStream.str());
		break;
	}
	return true;
}

uint32_t cpu::addAndSetFlags(uint32_t a, uint32_t b)
{
    int64_t sum = (int64_t)a+(int64_t)b;
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
	return a+b;
}

uint32_t cpu::subtractAndSetFlags(uint32_t a, uint32_t b)
{
    int64_t sum = (int64_t)a-(int64_t)b;
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
	//std::cout<<(uint32_t)a-(uint32_t)b<<"\n";
	return a-b;
}

uint64_t cpu::unsignedDivAndSetFlags(uint64_t a, uint64_t b, uint64_t& remainder)
{
	uint64_t division = a/b;
	remainder = a%b;
    return division;
}

uint64_t cpu::unsignedMulAndSetFlags(uint64_t a, uint64_t b)
{
    return a*b;
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
