#include "RegisterSet.h"

RegisterSet::RegisterSet():
	mode(RS_USER)
{
}

RegisterSet::~RegisterSet()
{
}

void RegisterSet::SetValue(unsigned int regId, u_int32_t value, DataWrapper *pcvalue)
{
	registers[regId] = value;
	if(regId == PC && pcvalue)
		pcvalue->Set(value);
}

u_int32_t RegisterSet::GetValue(unsigned int regId) const
{
	return registers[regId];
}

void RegisterSet::SetMode(ARMRegSet m)
{
	mode = m;
}

u_int32_t &RegisterSet::getReg(unsigned int regId)
{
	switch(mode)
	{

		//RS_USER,
		case RS_FIQ:
			if(regId >= R8 && regId <= LR)
				return registers[regId + R8_FIQ];
			if(regId == SPSR)
				return registers[SPSR_FIQ];
			break;
		
		case RS_IRQ:
			if(regId == SP || regId == LR)
				return registers[regId + SP_IRQ];
			if(regId == SPSR)
				return registers[SPSR_IRQ];
			break;

		default:
			break;
	}
	return registers[regId];

	/*
	RS_SUPERVISOR,
	RS_ABORT,
	RS_IRQ,
	RS_UNDEFINED,
	RS_COUNT
	*/
}

