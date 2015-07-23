#ifndef MEMORY_H
#define MEMORY_H

#include <sys/types.h>

enum MemoryAccess
{
	MA32,
	MA16,
	MA8
};

class Memory
{
	public:
		Memory(u_int32_t bAddress, u_int32_t eAddress);
		virtual ~Memory();

		virtual bool Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess = MA32) = 0;
		virtual bool Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess = MA32) = 0;

	protected:
		u_int32_t bAddress;
		u_int32_t eAddress;
		u_int32_t *data;
};

#endif

