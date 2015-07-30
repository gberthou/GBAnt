#include "sound.h"

Sound::Sound(u_int32_t bAddress, u_int32_t eAddress):
	Memory(bAddress, eAddress)
{
}

Sound::~Sound()
{
}

bool Sound::Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess)
{
	(void) value;
	(void) memAccess;
	if(address >= bAddress && address <= eAddress)
	{
		return true;
	}
	return false;
}

bool Sound::Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess)
{
	(void) value;
	(void) memAccess;
	if(address >= bAddress && address <= eAddress)
	{
		return true;
	}
	return false;
}
	
