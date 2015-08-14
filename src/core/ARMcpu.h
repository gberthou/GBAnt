#ifndef ARM_CPU_H
#define ARM_CPU_H

#include "RegisterSet.h"
#include "MemoryContainer.h"
#include "PhysicalMemory.h"
#include "DataWrapper.h"

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
		virtual void runStep(void);
		virtual void onClock(void);
		virtual bool interruptsEnabled(void);
		
		void executeInstruction(u_int32_t instruction);
		void alu(u_int8_t op, u_int8_t s, u_int8_t rd, u_int8_t rn, u_int32_t op2, StatusBit shiftCarry, DataWrapper &pcValue);
		void aluThumbRegister(u_int8_t op, u_int8_t rd, u_int8_t rs, DataWrapper &pcValue);
		void aluThumbImm(u_int8_t op, u_int8_t rd, u_int8_t nn, DataWrapper &pcValue);
		void executeInstructionThumb(u_int16_t instruction);
		void updateStatus(u_int32_t x, StatusBit carry, StatusBit overflow);
		bool testCondition(u_int8_t condition);

		bool thumbMode;
		RegisterSet regSet;
		u_int32_t cycles;

		// Memory
		u_int32_t baseAddress;
		MemoryContainer mem;
};

#endif

