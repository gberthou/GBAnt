#ifndef ARM_CPU_H
#define ARM_CPU_H

#include "RegisterSet.h"
#include "MemoryContainer.h"
#include "PhysicalMemory.h"

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

		virtual void Run(void);
	
	protected:
		void executeInstruction(u_int32_t instruction);
		void alu(u_int8_t op, u_int8_t s, u_int8_t rd, u_int8_t rn, u_int32_t op2, StatusBit shiftCarry);
		void aluThumbRegister(u_int8_t op, u_int8_t rd, u_int8_t rs);
		void aluThumbImm(u_int8_t op, u_int8_t rd, u_int8_t nn);
		bool executeInstructionThumb(u_int16_t instruction);
		void updateStatus(u_int32_t x, StatusBit carry, StatusBit overflow);
		bool testCondition(u_int8_t condition);

		bool thumbMode;
		RegisterSet regSet;
		
		// Memory
		u_int32_t baseAddress;
		MemoryContainer mem;
};

#endif

