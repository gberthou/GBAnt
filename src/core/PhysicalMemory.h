#ifndef PHYSICAL_MEMORY
#define PHYSICAL_MEMORY

#include <sys/types.h>

#include "Memory.h"

class PhysicalMemory : public Memory
{
	public:
		PhysicalMemory(u_int32_t bAddress, u_int32_t eAddress);
		virtual ~PhysicalMemory();

		virtual bool Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess = MA32);
		virtual bool Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess = MA32);

		bool CopyToMemory(u_int32_t dstAddress, const void *src, size_t size);
};

#endif

