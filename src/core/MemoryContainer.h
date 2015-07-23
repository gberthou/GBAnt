#ifndef MEMORY_CONTAINER
#define MEMORY_CONTAINER

#include <sys/types.h>
#include <vector>

#include "Memory.h"

class MemoryContainer : public Memory
{
	public:
		MemoryContainer(u_int32_t bAddress, u_int32_t eAddress);
		virtual ~MemoryContainer();

		virtual bool Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess = MA32);
		virtual bool Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess = MA32);

		void AddMemory(Memory *mem);

	protected:
		std::vector<Memory*> submems;
};

#endif

