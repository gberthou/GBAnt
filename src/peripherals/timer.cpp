#include "timer.h"

Timer::Timer(u_int32_t bAddress, u_int32_t eAddress):
	Memory(bAddress, eAddress)
{
}

Timer::~Timer()
{
}

bool Timer::Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess)
{
	(void) value;
	(void) memAccess;
	if(address >= bAddress && address <= eAddress)
	{
		return true;
	}
	return false;
}

bool Timer::Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess)
{
	(void) value;
	(void) memAccess;
	if(address >= bAddress && address <= eAddress)
	{
		return true;
	}
	return false;
}

