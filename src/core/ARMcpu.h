#ifndef ARM_CPU_H
#define ARM_CPU_H

#include "RegisterSet.h"
#include "MemoryContainer.h"
#include "PhysicalMemory.h"
#include "DataWrapper.h"
#include "irq.h"

enum StatusBit
{
	SB_0,
	SB_1,
	SB_UNCHANGED
};

class ARMcpu
{
	public:
		ARMcpu(u_int32_t baseAddress);
		virtual ~ARMcpu();

		virtual void RunNextInstruction(void);

	protected:
		virtual void onClock(void);
		virtual bool interruptsEnabled(void);
		virtual void biosCall(u_int8_t op) = 0; // svc calls
		
		void executeInstruction(u_int32_t instruction);
		void alu(u_int8_t op, u_int8_t s, u_int8_t rd, u_int8_t rn, u_int32_t op2, StatusBit shiftCarry, DataWrapper &pcValue);
		void aluThumbRegister(u_int8_t op, u_int8_t rd, u_int8_t rs, DataWrapper &pcValue);
		void aluThumbImm(u_int8_t op, u_int8_t rd, u_int8_t nn, DataWrapper &pcValue);
		void executeInstructionThumb(u_int16_t instruction);
		void updateStatus(u_int32_t x, StatusBit carry, StatusBit overflow);
		bool testCondition(u_int8_t condition);
		u_int32_t loadstore(unsigned int reg, u_int16_t registers, bool pclr, bool r16, bool load, bool increment, bool after, DataWrapper &pcwrapper);
		void halt(u_int32_t flagsToWaitFor);
		bool tryToResume(GBA_InterruptSource interruptSource);

		bool thumbMode;
		bool halted;
		u_int32_t haltFlags;
		RegisterSet regSet;
		u_int32_t cycles;

		// Memory
		u_int32_t baseAddress;
		MemoryContainer mem;
};

#endif

