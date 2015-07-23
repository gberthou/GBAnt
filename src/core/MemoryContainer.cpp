#include <iostream>
#include <cstdlib>

#include "MemoryContainer.h"
#include "utils.h"

MemoryContainer::MemoryContainer(u_int32_t bAddr, u_int32_t eAddr):
	Memory(bAddr, eAddr)
{
}

MemoryContainer::~MemoryContainer()
{
	for(std::vector<Memory*>::iterator it = submems.begin();
		it != submems.end();
		++it)
		delete *it;
}

bool MemoryContainer::Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess)
{
	if(address >= bAddress && address <= eAddress)
	{
		for(std::vector<Memory*>::iterator it = submems.begin();
			it != submems.end();
			++it)
			if((*it)->Write(address, value, memAccess))
				return true;
	}
	
	std::cerr << "User intended to write to non-memory @";
	PrintHex(address, 32, std::cerr);
	std::cerr << std::endl;
	exit(1);

	return false;
}

bool MemoryContainer::Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess)
{
	if(address >= bAddress && address <= eAddress)
	{
		for(std::vector<Memory*>::iterator it = submems.begin();
			it != submems.end();
			++it)
			if((*it)->Read(address, value, memAccess))
				return true;
	}
	
	std::cerr << "User intended to read to non-memory @";
	PrintHex(address, 32, std::cerr);
	std::cerr << std::endl;
	exit(1);

	return false;
}

void MemoryContainer::AddMemory(Memory *mem)
{
	submems.push_back(mem);
}

