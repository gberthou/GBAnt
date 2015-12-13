#include <iostream>
#include <cstring>

#include "PhysicalMemory.h"

PhysicalMemory::PhysicalMemory(u_int32_t bAddress, u_int32_t eAddress):
	Memory(bAddress, eAddress)
{
	data = new u_int32_t[eAddress - bAddress + 1];
}

PhysicalMemory::~PhysicalMemory()
{
	delete [] data;
}

bool PhysicalMemory::Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess)
{
	if(address >= bAddress && address <= eAddress)
	{
		switch(memAccess)
		{
			case MA8:
			{
				u_int8_t *data8 = reinterpret_cast<u_int8_t*>(data);
				data8[address - bAddress] = value;
				break;
			}

			case MA16:
			{
				u_int16_t *data16 = reinterpret_cast<u_int16_t*>(data);
				data16[(address - bAddress) >> 1] = value;
				break;
			}
			
			default: // MA32
				data[(address - bAddress) >> 2] = value;
				break;
		}
		return true;
	}
	return false;
}

bool PhysicalMemory::Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess)
{
	if(address >= bAddress && address <= eAddress)
	{
		switch(memAccess)
		{
			case MA8:
			{
				u_int8_t *data8 = reinterpret_cast<u_int8_t*>(data);
				value = data8[address - bAddress];
				break;
			}

			case MA16:
			{
				u_int16_t *data16 = reinterpret_cast<u_int16_t*>(data);
				value = data16[(address - bAddress) >> 1];
				break;
			}

			default: // MA32
				value = data[(address - bAddress) >> 2];
				break;
		}
		return true;
	}
	return false;
}

bool PhysicalMemory::CopyToMemory(u_int32_t dstAddress, const void *src, size_t size)
{
	if(size == 0)
		return true;

	if(dstAddress >= bAddress && dstAddress <= eAddress && dstAddress + size - 1 <= eAddress)
	{
		memcpy(data + ((dstAddress - bAddress) >> 2), src, size);
		return true;
	}
	return false;
}

