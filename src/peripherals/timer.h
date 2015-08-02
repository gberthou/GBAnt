#ifndef TIMER_H
#define TIMER_H

#include "../core/Memory.h"

class Timer : public Memory
{
	public:
		Timer(u_int32_t bAddress, u_int32_t eAddress);
		virtual ~Timer();

		virtual bool Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess = MA32);
		virtual bool Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess = MA32);
};

#endif

