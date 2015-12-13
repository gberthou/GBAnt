#ifndef MIRROR_MEMORY
#define MIRROR_MEMORY

#include <sys/types.h>

#include "Memory.h"
#include "MemoryContainer.h"

class MirrorMemory : public Memory
{
	public:
		MirrorMemory(u_int32_t bAddress, u_int32_t eAddress, u_int32_t mirroredBaseAddress, MemoryContainer &memContainer);
		virtual ~MirrorMemory();

		virtual bool Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess = MA32);
		virtual bool Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess = MA32);

	protected:
		u_int32_t mirroredBaseAddress;
		MemoryContainer &memContainer;
};

#endif

