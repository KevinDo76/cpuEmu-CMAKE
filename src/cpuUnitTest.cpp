#include "cpuUnitTest.h"
#include "cpu.h"
#include <vector>
#include <iostream>
#include <stdint.h>

//https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";



static inline bool is_base64(char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::vector<char> base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  char char_array_4[4], char_array_3[3];
  std::vector<char> ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
          ret.push_back(char_array_3[i]);
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
  }

  return ret;
}





void cpuUnitTest()
{
    std::vector<std::pair<bool (*)(), std::string>> unitFunctions
    {
        {checkMov1, "checkMov1"},
        {checkJmpImm1, "checkJmpImm1"},
        {checkPopPush1, "checkPopPush1"},
        {checkPopregPushreg1, "checkPopregPushreg1"},
        {checkInc1, "checkInc1"},
        {checkDec1, "checkDec1"},
        {checkAdd1, "checkAdd1"},
        {checkSub1, "checkSub1"},
        {checkAddOverflow2, "checkAddOverflow2"},
        {checkSubUnderflow2, "checkSubUnderflow2"},
        {checkAnd1, "checkAnd1"},
        {checkXor1, "checkXor1"},
        {checkOr1, "checkOr1"},
        {checkCallRet1, "checkCallRet1"},
        {checkJmp1, "checkJmp1"}
        
    };
    for (int i=0;i<unitFunctions.size();i++)
    {
        if (unitFunctions[i].first())
        {
            std::cout<<"OK "<<unitFunctions[i].second<<"\n";
        } else {
            std::cout<<"FAIL "<<unitFunctions[i].second<<"\n";
        }
    }
}

bool checkMov1()
{
    cpu CPU;
    std::vector<char> cpuInstruction = base64_decode("GQAAAAAAAADvvq3eAAAAABkAAAABAAAAzwQAAAAAAAAZAAAAAgAAAK3e774AAAAAGQAAAAMAAADJAAAAAAAAAA==");
    CPU.loadBinaryArray(cpuInstruction);

    CPU.clockTick();
    CPU.clockTick();
    CPU.clockTick();
    CPU.clockTick();

    uint32_t ra = CPU.readGeneralRegister(0);
    uint32_t rb = CPU.readGeneralRegister(1);
    uint32_t rc = CPU.readGeneralRegister(2);
    uint32_t rd = CPU.readGeneralRegister(3);

    if (ra == 0xdeadbeef && rb == 1231 && rc == 0xbeefdead && CPU.readGeneralRegister(3) == 201)
    {
        return true;
    }
    
    return false;
}

bool checkJmpImm1()
{
  cpu CPU;
  std::vector<char> cpuInstruction = base64_decode("HAAAACAAAAAAAAAAAAAAABkAAAAAAAAAqgoAAAAAAAAZAAAAAAAAAO++rd4AAAAA=");
  CPU.loadBinaryArray(cpuInstruction);

  CPU.clockTick();
  CPU.clockTick();

  uint32_t ra = CPU.readGeneralRegister(0);
  if (ra == 0xdeadbeef)
  {
      return true;
  }
  
  return false;
}

bool checkPopPush1()
{
  cpu CPU;
  std::vector<char> cpuInstruction = base64_decode("GQAAAAYAAAAAMAAAAAAAABkAAAAFAAAAAAAAAAAAAAAZAAAAAAAAAK3e774AAAAABAAAAAAAAAAAAAAAAAAAAAMAAAABAAAAAAAAAAAAAAA==");
  CPU.loadBinaryArray(cpuInstruction);

  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();

  uint32_t rb = CPU.readGeneralRegister(1);

  if (rb == 0xbeefdead)
  {
    return true;
  }

  return false;
}

bool checkPopregPushreg1()
{
  cpu CPU;
  std::vector<char> cpuInstruction = base64_decode("GQAAAAYAAAAAMAAAAAAAABkAAAAFAAAAAAAAAAAAAAAZAAAAAAAAAK3e774AAAAAGQAAAAEAAADJAAAAAAAAABkAAAACAAAAExIBAAAAAAAZAAAAAwAAADYIAAAAAAAAJgAAAAAAAAAAAAAAAAAAABkAAAAAAAAAAAAAAAAAAAAZAAAAAQAAAAAAAAAAAAAAGQAAAAIAAAAAAAAAAAAAABkAAAADAAAAAAAAAAAAAAAnAAAAAAAAAAAAAAAAAAAA");
  CPU.loadBinaryArray(cpuInstruction);

  for (int i=0;i<12;i++)
  {
    CPU.clockTick();
  }

  uint32_t ra = CPU.readGeneralRegister(0);
  uint32_t rb = CPU.readGeneralRegister(1);
  uint32_t rc = CPU.readGeneralRegister(2);
  uint32_t rd = CPU.readGeneralRegister(3);

  if (ra == 0xbeefdead && rb == 201 && rc == 0x11213 && CPU.readGeneralRegister(3) == 2102)
  {
    return true;
  }
  
  return false;
}

bool checkInc1()
{
  cpu CPU;
  std::vector<char> cpuInstruction = base64_decode("GQAAAAAAAAAAAAAAAAAAACQAAAAAAAAAAAAAAAAAAAAkAAAAAAAAAAAAAAAAAAAAJAAAAAAAAAAAAAAAAAAAAA==");
  CPU.loadBinaryArray(cpuInstruction);

  for (int i=0;i<12;i++)
  {
    CPU.clockTick();
  }

  uint32_t ra = CPU.readGeneralRegister(0);

  if (ra == 3)
  {
    return true;
  }
  
  return false;
}

bool checkDec1()
{
  cpu CPU;
  std::vector<char> cpuInstruction = base64_decode("GQAAAAAAAAACAAAAAAAAACUAAAAAAAAAAAAAAAAAAAAlAAAAAAAAAAAAAAAAAAAAJQAAAAAAAAAAAAAAAAAAAA==");
  CPU.loadBinaryArray(cpuInstruction);

  for (int i=0;i<12;i++)
  {
    CPU.clockTick();
  }

  uint32_t ra = CPU.readGeneralRegister(0);

  if (ra == UINT32_MAX)
  {
    return true;
  }
  
  return false;
}

bool checkAdd1()
{
  cpu CPU;
  std::vector<char> cpuInstruction = base64_decode("GQAAAAAAAAAiAAAAAAAAABkAAAABAAAAIwAAAAAAAAARAAAAAAAAAAEAAAAAAAAA");
  CPU.loadBinaryArray(cpuInstruction);

  for (int i=0;i<12;i++)
  {
    CPU.clockTick();
  }

  uint32_t ra = CPU.readGeneralRegister(0);

  if (ra == 69)
  {
    return true;
  }
  
  return false;
}

bool checkAddOverflow2()
{
  cpu CPU;
  std::vector<char> cpuInstruction = base64_decode("GQAAAAAAAAD/////AAAAABkAAAABAAAAAQAAAAAAAAARAAAAAAAAAAEAAAAAAAAA");
  CPU.loadBinaryArray(cpuInstruction);

  for (int i=0;i<12;i++)
  {
    CPU.clockTick();
  }

  uint32_t ra = CPU.readGeneralRegister(0);

  if (ra == 0 && CPU.readGeneralRegister(7) & 4)
  {
    return true;
  }
  
  return false;
}

bool checkAnd1()
{
  cpu CPU;
  std::vector<char> cpuInstruction = base64_decode("GQAAAAAAAADvvq3eAAAAABkAAAABAAAArd7vvgAAAAAXAAAAAAAAAAEAAAAAAAAA");
  CPU.loadBinaryArray(cpuInstruction);

  
  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();

  uint32_t ra = CPU.readGeneralRegister(0);

  if (ra == 0x9EAD9EAD)
  {
    return true;
  }
  
  return false;
}

bool checkXor1()
{
  cpu CPU;
  std::vector<char> cpuInstruction = base64_decode("GQAAAAAAAADvvq3eAAAAABkAAAABAAAArd7vvgAAAAAWAAAAAAAAAAEAAAAAAAAA");
  CPU.loadBinaryArray(cpuInstruction);

  
  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();

  uint32_t ra = CPU.readGeneralRegister(0);

  if (ra == 0x60426042)
  {
    return true;
  }
  
  return false;
}

bool checkOr1()
{
  cpu CPU;
  std::vector<char> cpuInstruction = base64_decode("GQAAAAAAAADvvq3eAAAAABkAAAABAAAArd7vvgAAAAAYAAAAAAAAAAEAAAAAAAAA");
  CPU.loadBinaryArray(cpuInstruction);

  
  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();

  uint32_t ra = CPU.readGeneralRegister(0);

  if (ra == 0xFEEFFEEF)
  {
    return true;
  }
  
  return false;
}

bool checkCallRet1()
{
  cpu CPU;
  std::vector<char> cpuInstruction = base64_decode("GQAAAAYAAAAAMAAAAAAAABkAAAAFAAAAAAAAAAAAAAABAAAAcAAAAAAAAAAAAAAAGQAAAAAAAAAgAAAAAAAAABkAAAABAAAAyQAAAAAAAAAZAAAAAAAAAOcAAAAAAAAAGQAAAAEAAACuIgAAAAAAABkAAAAAAAAA776t3gAAAAAZAAAAAQAAAK3e774AAAAAAgAAAAAAAAAAAAAAAAAAAA==");
  CPU.loadBinaryArray(cpuInstruction);

  
  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();

  uint32_t ra = CPU.readGeneralRegister(0);
  uint32_t rb = CPU.readGeneralRegister(1);

  if (ra != 0xdeadbeef || rb != 0xbeefdead)
  {
    return false;
  }

  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();

  ra = CPU.readGeneralRegister(0);
  rb = CPU.readGeneralRegister(1);

  if (ra != 0x20 || rb != 201)
  {
    return false;
  }
  
  return true;
}

bool checkJmp1()
{
  cpu CPU;
  std::vector<char> cpuInstruction = base64_decode("GQAAAAYAAAAAMAAAAAAAABkAAAAFAAAAAAAAAAAAAAAZAAAAAAAAAHAAAAAAAAAAGwAAAAAAAAAAAAAAAAAAABkAAAAAAAAAIwAAAAAAAAAZAAAAAQAAANUAAAAAAAAAGQAAAAIAAADa2goAAAAAABkAAAAAAAAAMQIAAAAAAAAZAAAAAQAAAFMIAAAAAAAAGQAAAAIAAADvvq3eAAAAAA==");
  CPU.loadBinaryArray(cpuInstruction);

  
  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();

  uint32_t ra = CPU.readGeneralRegister(0);
  uint32_t rb = CPU.readGeneralRegister(1);
  uint32_t rc = CPU.readGeneralRegister(2);

  if (ra == 0x231 && rb == 2131 && rc == 0xdeadbeef)
  {
    return true;
  }
  
  return false;
}

bool checkSub1()
{
  cpu CPU;
  std::vector<char> cpuInstruction = base64_decode("GQAAAAAAAAAjAAAAAAAAABkAAAABAAAAIwAAAAAAAAAoAAAAAAAAAAEAAAAAAAAA");
  CPU.loadBinaryArray(cpuInstruction);

  
  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();

  uint32_t ra = CPU.readGeneralRegister(0);

  if (ra == 0)
  {
    return true;
  }
  
  return false;
}

bool checkSubUnderflow2()
{
  cpu CPU;
  std::vector<char> cpuInstruction = base64_decode("GQAAAAAAAAAAAAAAAAAAABkAAAABAAAAAQAAAAAAAAAoAAAAAAAAAAEAAAAAAAAA");
  CPU.loadBinaryArray(cpuInstruction);

  
  CPU.clockTick();
  CPU.clockTick();
  CPU.clockTick();

  uint32_t ra = CPU.readGeneralRegister(0);

  if (ra == UINT32_MAX  && CPU.readGeneralRegister(7) & 4)
  {
    return true;
  }
  
  return false;
}
