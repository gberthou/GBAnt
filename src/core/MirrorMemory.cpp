#include <iostream>
#include <cstring>

#include "MirrorMemory.h"

MirrorMemory::MirrorMemory(u_int32_t bAddress, u_int32_t eAddress, u_int32_t mirroredbAddress, MemoryContainer &mContainer):
	Memory(bAddress, eAddress),
	mirroredBaseAddress(mirroredbAddress),
	memContainer(mContainer)
{
	data = 0;
}

MirrorMemory::~MirrorMemory()
{
}

bool MirrorMemory::Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess)
{
	if(address >= bAddress && address <= eAddress)
		return memContainer.Write(address + mirroredBaseAddress - bAddress, value, memAccess);
	return false;
}

bool MirrorMemory::Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess)
{
	if(address >= bAddress && address <= eAddress)
		return memContainer.Read(address + mirroredBaseAddress - bAddress, value, memAccess);
	return false;
}

