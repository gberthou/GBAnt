#include <iostream>

#include "utils.h"
#include "RegisterSet.h"

void PrintHex(u_int32_t x, unsigned int bitcount, std::ostream &out)
{
	unsigned int i = bitcount == 16 ? 4 : 8;
	out << "0x";
	while(i--)
	{
		char n = (x >> (4 * i)) & 0xF;
		if(n < 10)
			out << (char)('0' + n);
		else
			out << (char)('a' + n - 10);
	}
}

void PrintRegister(u_int8_t reg)
{
	switch(reg)
	{
		case SP:
			std::cout << "sp";
			return;
		case LR:
			std::cout << "lr";
			return;
		case PC:
			std::cout << "pc";
			return;
		case CPSR:
			std::cout << "cpsr";
			return;
		case SPSR:
			std::cout << "spsr";
			return;
		default:
			std::cout << 'r' << (int) reg;
			return;
	}
}

void PrintCondition(u_int8_t condition)
{
	switch(condition)
	{
		case 0x0: // EQ
			std::cout << "eq";
			return;
		case 0x1: // NE
			std::cout << "ne";
			return;
		case 0x2: // CS
			std::cout << "cs";
			return;
		case 0x3: // CC
			std::cout << "cc";
			return;
		case 0x4: // MI
			std::cout << "mi";
			return;
		case 0x5: // PL
			std::cout << "pl";
			return;
		case 0x6: // VS
			std::cout << "vs";
			return;
		case 0x7: // VC
			std::cout << "vc";
			return;
		case 0x8: // HI
			std::cout << "hi";
			return;
		case 0x9: // LS
			std::cout << "ls";
			return;
		case 0xA: // GE
			std::cout << "ge";
			return;
		case 0xB: // LT
			std::cout << "lt";
			return;
		case 0xC: // GT
			std::cout << "gt";
			return;
		case 0xD: // LE
			std::cout << "le";
			return;
		case 0xE: // AL
		case 0xF: // NV
			return;
		default:
			return;
	}
}

int32_t Unsigned2Signed(u_int32_t x, unsigned int bitcount)
{
	if((x >> bitcount) & 1) // Negative
		return -(((~x) + 1) & ((1 << bitcount) - 1));
	
	// Positive
	return x;
}

