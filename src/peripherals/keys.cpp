#include <cstring>

#include "keys.h"
		
Keys::Keys(u_int32_t bAddress, u_int32_t eAddress):
	Memory(bAddress, eAddress)
{
	memset(&control, 0, sizeof(control));
}

Keys::~Keys()
{
}
		
bool Keys::Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess)
{
	if(address >= bAddress && address <= eAddress)
	{
		if(memAccess == MA16)
		{
			u_int16_t *data = reinterpret_cast<u_int16_t*>(&control);
			data[(address - bAddress) / 2] = value;
		}
		else // MA32
		{
			u_int32_t *data = reinterpret_cast<u_int32_t*>(&control);
			data[(address - bAddress) / 4] = value;
		}
		return true;
	}
	return false;
}

bool Keys::Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess)
{
	if(address >= bAddress && address <= eAddress)
	{
		if(memAccess == MA16)
		{
			u_int16_t *data = reinterpret_cast<u_int16_t*>(&control);
			value = *(data + (address - bAddress) / 2);
		}
		else // MA32
		{
			u_int32_t *data = reinterpret_cast<u_int32_t*>(&control);
			value = *(data + (address - bAddress) / 4);
		}

		return true;
	}
	return false;
}

