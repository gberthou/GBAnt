#ifndef KEYS_H
#define KEYS_H

#include "../core/Memory.h"
#include "../core/utils.h"

struct KeyControl
{
	u_int16_t keyinput;
	u_int16_t keycnt;
};

class GBAcpu;

class Keys : public Memory
{
	public:
		Keys(u_int32_t bAddress, u_int32_t eAddress);
		virtual ~Keys();
		
		virtual bool Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess = MA32);
		virtual bool Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess = MA32);

	private:
		KeyControl control;
};

#endif

