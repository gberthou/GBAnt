#ifndef REGISTER_SET_H
#define REGISTER_SET_H

#include <sys/types.h>

#include "DataWrapper.h"

enum ARMRegSet
{
	RS_USER,
	RS_FIQ,
	RS_SVC,
	RS_ABORT,
	RS_IRQ,
	RS_UNDEFINED,
	RS_COUNT
};

enum ARMRegister
{
	// Common + System/User
	R0,
	R1,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
	R8,
	R9,
	R10,
	R11,
	R12,
	SP,
	LR,
	PC,
	CPSR,
	SPSR = 17,

	// FIQ specific
	R8_FIQ = 17,
	R9_FIQ,
	R10_FIQ,
	R11_FIQ,
	R12_FIQ,
	SP_FIQ,
	LR_FIQ,
	SPSR_FIQ,

	// Supervisor specific
	SP_SVC,
	LR_SVC,
	SPSR_SVC,

	// Abort specific
	SP_ABT,
	LR_ABT,
	SPSR_ABT,

	// IRQ specific
	SP_IRQ,
	LR_IRQ,
	SPSR_IRQ,

	// Undefined specific
	SP_UND,
	LR_UND,
	SPSR_UND,

	REGISTER_COUNT
};

class RegisterSet
{
	public:
		RegisterSet();
		virtual ~RegisterSet();

		void SetValue(unsigned int regId, u_int32_t value, DataWrapper *pcvalue);
		u_int32_t GetValue(unsigned int regId) const;
		
		void SetMode(ARMRegSet mode);

	protected:
		u_int32_t &getReg(unsigned int regId);

		u_int32_t registers[REGISTER_COUNT];
		ARMRegSet mode;
};

#endif

