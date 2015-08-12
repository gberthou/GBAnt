#include <iostream>
#include <cstdio>
#include <cstdlib>

#include "ARMcpu.h"
#include "Instruction.h"
#include "InstructionThumb.h"
#include "utils.h"

#define N_FLAG 31
#define Z_FLAG 30
#define C_FLAG 29
#define V_FLAG 28
#define Q_FLAG 27

inline StatusBit carryShiftRight(u_int32_t x, u_int32_t n)
{
	return ((x >> (n-1)) & 1) ? SB_1 : SB_0;
}

inline StatusBit carryShiftLeft(u_int32_t x, u_int32_t n)
{
	return ((x >> (32-n)) & 1) ? SB_1 : SB_0;
}

inline u_int32_t lsl(u_int32_t x, u_int32_t n)
{
	return x << n;
}

inline u_int32_t lsr(u_int32_t x, u_int32_t n)
{
	return x >> n;
}

inline u_int32_t asr(u_int32_t x, u_int32_t n)
{
	return (x & (1 << 31)) | ((x & ~(1 << 31)) >> n);
}

inline u_int32_t ror(u_int32_t x, u_int32_t n)
{
	return (x << (32 - n)) | (x >> n);
}

static void printRegisters(const RegisterSet &regSet, u_int8_t rlist)
{
	for(unsigned i = 0; i < 8; ++i)
	{
		if((rlist >> i) & 1)
		{
			PrintRegister(i);
			std::cout << ": ";
			PrintHex(regSet.GetValue(i));
			std::cout << std::endl;
		}
	}
}

ARMcpu::ARMcpu(u_int32_t bAddress):
	thumbMode(false),
	cycles(0),
	baseAddress(bAddress),
	mem(0, 0x0FFFFFFF)
{
}

ARMcpu::~ARMcpu()
{
}

void ARMcpu::Run(void)
{
	bool lock = false;
	for(;;)
	{
		u_int32_t pcValue = regSet.GetValue(PC);

		if(!thumbMode)
		{
			u_int32_t inst;

			mem.Read(pcValue, inst);

			std::cout << "@";
			PrintHex(pcValue);
			std::cout << ": ";
			executeInstruction(inst);
			std::cout << std::endl;
		}
		else
		{
			u_int32_t inst;

			mem.Read(pcValue, inst, MA16);

			std::cout << "@";
			PrintHex(pcValue);
			std::cout << ": ";
			if(!executeInstructionThumb(inst))
				lock = true;
			std::cout << std::endl;
		}

		onClock();

		if(lock)
			getchar();
	}
}

void ARMcpu::onClock(void)
{
	++cycles;
}

void ARMcpu::executeInstruction(u_int32_t instruction)
{
	Instruction inst;

	PrintHex(instruction);
	std::cout << std::endl;

	if(DecodeInstruction(instruction, inst))
	{
		u_int32_t pcValue = regSet.GetValue(PC);

		PrintInstruction(inst);
		std::cout << std::endl;

		switch(inst.type)
		{
			case IT_DATA_PROC1:
				if(testCondition(inst.data.dp1.cond))
				{
					InstDataProc1 *dp1 = &inst.data.dp1;
					u_int32_t rmv = regSet.GetValue(dp1->rm);
					u_int32_t op2;
					StatusBit scarry;

					switch(dp1->typ)
					{
						case 1:
							op2 = lsr(rmv, dp1->shift);
							scarry = carryShiftRight(rmv, dp1->shift);
							break;
						case 2:
							op2 = asr(rmv, dp1->shift);
							scarry = carryShiftRight(rmv, dp1->shift);
							break;
						case 3:
							op2 = ror(rmv, dp1->shift);
							scarry = carryShiftRight(rmv, dp1->shift);
							break;
						default:
							op2 = lsl(rmv, dp1->shift);
							scarry = carryShiftLeft(rmv, dp1->shift);
							break;
					}
					alu(dp1->op, dp1->s, dp1->rd, dp1->rn, op2, scarry);
				}
				pcValue += 4;
				break;
			case IT_DATA_PROC2:
				if(testCondition(inst.data.dp2.cond))
				{
					InstDataProc2 *dp2 = &inst.data.dp2;
					u_int32_t rmv = regSet.GetValue(dp2->rm);
					u_int32_t rsv = regSet.GetValue(dp2->rs) & 0xFF;
					u_int32_t op2;
					StatusBit scarry;

					switch(dp2->typ)
					{
						case 1:
							op2 = lsr(rmv, rsv);
							scarry = carryShiftRight(rmv, rsv);
							break;
						case 2:
							op2 = asr(rmv, rsv);
							scarry = carryShiftRight(rmv, rsv);
							break;
						case 3:
							op2 = ror(rmv, rsv);
							scarry = carryShiftRight(rmv, rsv);
							break;
						default:
							op2 = lsl(rmv, rsv);
							scarry = carryShiftLeft(rmv, rsv);
							break;
					}
					alu(dp2->op, dp2->s, dp2->rd, dp2->rn, op2, scarry);
				}
				pcValue += 4;
				break;

			case IT_DATA_PROC3:
				if(testCondition(inst.data.dp3.cond))
				{
					InstDataProc3 *dp3 = &inst.data.dp3;
					u_int32_t op2 = ror(dp3->immediate, dp3->shift);
					StatusBit scarry = carryShiftRight(dp3->immediate, dp3->shift);
					alu(dp3->op, dp3->s, dp3->rd, dp3->rn, op2, scarry);
				}
				pcValue += 4;
				break;

			case IT_PSR_REG:
				if(testCondition(inst.data.psrr.cond))
				{
					InstPSRReg *psrr = &inst.data.psrr;
					u_int32_t value = regSet.GetValue(psrr->p ? SPSR : CPSR);

					if(psrr->l) // MSR
					{
						u_int32_t rdv = regSet.GetValue(psrr->rd);
						for(unsigned int i = 0; i < 4; ++i)
						{
							if(psrr->field & (1 << i))
							{
								u_int32_t mask = 0xFF << (i << 3);
								// Clear field
								value &= ~mask;
								// Set field
								value |= (rdv & mask);
							}
						}
					}
					else // MRS
					{
						// TODO
					}
				}
				pcValue += 4;
				break;
			
			case IT_BRANCH:
				if(!testCondition(inst.data.b.cond))
					pcValue += 4;
				else
				{
					if(inst.data.b.l) // link
						regSet.SetValue(LR, pcValue + 4);
					pcValue += 8 + inst.data.b.offset;
				}
				break;

			case IT_BRANCH_EX:
				if(!testCondition(inst.data.bx.cond))
					pcValue += 4;
				else
				{
					u_int32_t rnv = regSet.GetValue(inst.data.bx.rn);
					if(inst.data.bx.l) // link
						regSet.SetValue(LR, pcValue + 4);

					pcValue = rnv & 0xFFFFFFFE;
					thumbMode = ((rnv & 1) == 1);
					
					if(thumbMode)
						std::cout << "Thumb mode enabled" << std::endl;
				}
				break;

			case IT_TRANS_IMM9:
				if(testCondition(inst.data.ti9.cond))
				{
					u_int32_t address = regSet.GetValue(inst.data.ti9.rn);
					u_int32_t value;

					// TODO: manage byte access

					if(inst.data.ti9.b)
						std::cout << "Zut... byte :(" << std::endl;

					if(inst.data.ti9.p) // Pre
					{
						if(inst.data.ti9.u) // Up => add offset
							address += inst.data.ti9.offset;
						else // Down => subtract offset
							address -= inst.data.ti9.offset;
					}

					if(inst.data.ti9.rn == PC)
						address += 8;

					if(inst.data.ti9.l) // LDR
					{
						if(!mem.Read(address, value))
						{
							std::cerr << "Cannot read at address ";
							PrintHex(address);
							std::cerr << std::endl;
							return;
						}
						std::cout << "Loading value ";
						PrintHex(value);
						std::cout << std::endl;
						regSet.SetValue(inst.data.ti9.rd, value);
					}
					else // STR
					{
						value = regSet.GetValue(inst.data.ti9.rd);
						if(!mem.Write(address, value))
						{
							std::cerr << "Cannot write at address ";
							PrintHex(address);
							std::cerr << std::endl;
							return;
						}
					}
				
					if(!inst.data.ti9.p) // Pre
					{
						if(inst.data.ti9.u) // Up => add offset
							address += inst.data.ti9.offset;
						else // Down => subtract offset
							address -= inst.data.ti9.offset;
					}
				
					if(!inst.data.ti9.p || inst.data.ti9.w) // Write-back
						regSet.SetValue(inst.data.ti9.rn, address);
				}

				pcValue += 4;
				break;

			case IT_BLOCK_TRANS:
			{
				InstBlockTrans *bt = &inst.data.bt;
				if(testCondition(bt->cond))
				{
					unsigned int delta = bt->u ? 4 : -4;
					unsigned int rnv = regSet.GetValue(bt->rn);
					u_int32_t value;

					// TODO: PSR bit
					
					if(bt->l) // LDM
					{
						for(unsigned int i = 0; i < 16; ++i)
						{
							if((bt->rlist >> i) & 1)
							{
								if(bt->p)
									rnv += delta;

								mem.Read(rnv, value);
								regSet.SetValue(i, value);

								if(!bt->p)
									rnv += delta;
							}
						}
					}
					else // STM
					{
						for(unsigned int i = 0; i < 16; ++i)
						{
							if((bt->rlist >> i) & 1)
							{
								if(bt->p)
									rnv += delta;

								mem.Write(rnv, regSet.GetValue(i));

								if(!bt->p)
									rnv += delta;
							}
						}
					}

					if(bt->w) // Write-back
						regSet.SetValue(bt->rn, rnv);
				}
				pcValue += 4;
				break;
			}

			default:
				std::cerr << "Instruction not supported yet" << std::endl;
				pcValue += 4;
				break;
		}

		regSet.SetValue(PC, pcValue);
	}
	else
	{
		std::cerr << "Unknown instruction..." << std::endl;
		exit(0);
	}
}

void ARMcpu::alu(u_int8_t op, u_int8_t s, u_int8_t rd, u_int8_t rn, u_int32_t op2, StatusBit shiftCarry)
{
	u_int32_t result;

	StatusBit carry = SB_UNCHANGED;
	StatusBit overflow = SB_UNCHANGED;

	switch(op)
	{
		case 0x0: // AND
		{
			u_int32_t rnv = regSet.GetValue(rn);
			result = rnv & op2;
			carry = shiftCarry;
			break;
		}
		case 0x1: // EOR
		{
			u_int32_t rnv = regSet.GetValue(rn);
			result = rnv ^ op2;
			carry = shiftCarry;
			break;
		}
		case 0x2: // SUB
		{
			u_int32_t rnv = regSet.GetValue(rn);
			result = rnv - op2;
			if(s)
				carry = result > rnv ? SB_1 : SB_0;
			break;
		}
		case 0x3: // RSB
		{
			u_int32_t rnv = regSet.GetValue(rn);
			result = op2 - rnv;
			if(s)
				carry = result > rnv ? SB_1 : SB_0;
			break;
		}
		case 0x4: // ADD
		{
			u_int32_t rnv = regSet.GetValue(rn);
			result = rnv + op2;
			if(s)
				carry = result < rnv ? SB_1 : SB_0;
			break;
		}
		case 0x5: // ADC
		{
			u_int32_t rnv = regSet.GetValue(rn);
			u_int32_t cpsr = regSet.GetValue(CPSR);
			result = rnv + op2 + ((cpsr >> 29) & 1);
			if(s)
				carry = result < rnv ? SB_1 : SB_0;
			break;
		}
		case 0x6: // SBC
		{
			u_int32_t rnv = regSet.GetValue(rn);
			u_int32_t cpsr = regSet.GetValue(CPSR);
			result = rnv - op2 - 1 + ((cpsr >> 29) & 1);
			if(s)
				carry = result > rnv ? SB_1 : SB_0;
			break;
		}
		case 0x7: // RSC
		{
			u_int32_t rnv = regSet.GetValue(rn);
			u_int32_t cpsr = regSet.GetValue(CPSR);
			result = op2 - rnv - 1 + ((cpsr >> 29) & 1);
			if(s)
				carry = result > rnv ? SB_1 : SB_0;
			break;
		}
		case 0x8: // TST
		{
			u_int32_t rnv = regSet.GetValue(rn);
			result = rnv & op2;
			carry = shiftCarry;
			break;
		}
		case 0x9: // TEQ
		{
			u_int32_t rnv = regSet.GetValue(rn);
			result = rnv ^ op2;
			carry = shiftCarry;
			break;
		}
		case 0xA: // CMP
		{
			u_int32_t rnv = regSet.GetValue(rn);
			result = rnv - op2;
			if(s)
				carry = result > rnv ? SB_1 : SB_0;
			break;
		}
		case 0xB: // CMN
		{
			u_int32_t rnv = regSet.GetValue(rn);
			result = rnv + op2;
			if(s)
				carry = result < rnv ? SB_1 : SB_0;
			break;
		}
		case 0xC: // ORR
		{
			u_int32_t rnv = regSet.GetValue(rn);
			result = rnv | op2;
			carry = shiftCarry;
			break;
		}
		case 0xD: // MOV
			result = op2;
			carry = shiftCarry;
			break;
		case 0xE: // BIC
		{
			u_int32_t rnv = regSet.GetValue(rn);
			result = rnv & (~op2);
			carry = shiftCarry;
			break;
		}
		case 0xF: // MVN
			result = ~op2;
			carry = shiftCarry;
			break;
		default:
			break;
	}
	
	if(op <= 0x7 || (op >= 0xC && op <= 0xF))
	{
		regSet.SetValue(rd, result);
		if(s)
			updateStatus(result, carry, overflow);
	}
	else
		updateStatus(result, carry, overflow);
}

void ARMcpu::aluThumbRegister(u_int8_t op, u_int8_t rd, u_int8_t rs)
{
	u_int32_t rdv = regSet.GetValue(rd);
	u_int32_t rsv = regSet.GetValue(rs);
	u_int32_t result;
	StatusBit carry = SB_UNCHANGED;
	StatusBit overflow = SB_UNCHANGED;
	
	switch(op)
	{
		case 0x0: // AND
			result = rdv & rsv;
			break;
		case 0x1: // EOR
			result = rdv ^ rsv;
			break;
		case 0x2: // LSL
			result = lsl(rdv, rsv & 0xFF);
			carry = carryShiftLeft(rdv, rsv & 0xFF);
			break;
		case 0x3: // LSR
			result = lsr(rdv, rsv & 0xFF);
			carry = carryShiftRight(rdv, rsv & 0xFF);
			break;
		case 0x4: // ASR
			result = asr(rdv, rsv & 0xFF);
			carry = carryShiftRight(rdv, rsv & 0xFF);
			break;
		case 0x5: // ADC
		{
			u_int32_t cpsr = regSet.GetValue(CPSR);
			result = rdv + rsv + ((cpsr >> 29) & 1);
			carry = result < rdv ? SB_1 : SB_0;
			break;
		}
		case 0x6: // SBC
		{
			u_int32_t cpsr = regSet.GetValue(CPSR);
			result = rdv - rsv - 1 + ((cpsr >> 29) & 1);
			carry = result > rdv ? SB_1 : SB_0;
			break;
		}
		case 0x7: // ROR
			result = ror(rdv, rsv & 0xFF);
			carry = carryShiftRight(rdv, rsv & 0xFF);
			break;
		case 0x8: // TST
			result = rdv & rsv;
			break;
		case 0x9: // NEG
			// TODO: Update carry flag (and overflow?)
			result = ~rdv;
			break;
		case 0xA: // CMP
			result = rdv - rsv;
			carry = result > rdv ? SB_1 : SB_0;
			break;
		case 0xB: // CMN
			result = rdv + rsv;
			carry = result < rdv ? SB_1 : SB_0;
			break;
		case 0xC: // ORR
			result = rdv | rsv;
			break;
		case 0xD: // MUL
			result = rdv * rsv;
			break;
		case 0xE: //BIC
			result = rdv & (~rsv);
			break;
		case 0xF: // MVN
			result = ~rsv;
			break;
		default: // Should not happen
			break;
	}
	if(op != 0x8 && op != 0xA && op != 0xB)
		regSet.SetValue(rd, result);

	updateStatus(result, carry, overflow);
}

void ARMcpu::aluThumbImm(u_int8_t op, u_int8_t rd, u_int8_t nn)
{
	u_int32_t result;
	StatusBit carry = SB_UNCHANGED;
	StatusBit overflow = SB_UNCHANGED;
	
	switch(op)
	{
		case 0: // MOV
			result = nn;
			break;

		case 1: // CMP
		case 3: // SUB
		{
			u_int32_t rdv = regSet.GetValue(rd);
			result = rdv - nn;
			carry = result > rdv ? SB_1 : SB_0;
			break;
		}

		case 2: // ADD
			result = regSet.GetValue(rd) + nn;
			carry = result > nn ? SB_1 : SB_0;
			break;

		default:
			break;
	}

	if(op != 1) // Not CMP
		regSet.SetValue(rd, result);
	
	updateStatus(result, carry, overflow);
}

void ARMcpu::updateStatus(u_int32_t x, StatusBit carry, StatusBit overflow)
{
	u_int32_t cpsr = regSet.GetValue(CPSR);

	cpsr &= ~(3 << 30); // Clear NZ bits
	cpsr |= x & (1 << 31); // N
	cpsr |= (x == 0) << 30; // Z

	if(carry != SB_UNCHANGED)
	{
		cpsr &= ~(1 << 29);
		cpsr |= (carry << 29);
	}

	if(overflow != SB_UNCHANGED)
	{
		cpsr &= ~(1 << 28);
		cpsr |= (overflow << 28);
	}

	regSet.SetValue(CPSR, cpsr);
}

bool ARMcpu::testCondition(u_int8_t condition)
{
	u_int32_t cpsr = regSet.GetValue(CPSR);
	switch(condition)
	{
		case 0x0: // EQ
			return (cpsr >> Z_FLAG) & 1; // Z flag
		case 0x1: // NE
			return (cpsr & (1 << 30)) == 0; // Z flag
		case 0x2: // CS
			return (cpsr & (1 << 29)) != 0; // C flag
		case 0x3: // CC
			return (cpsr & (1 << 29)) == 0; // C flag
		case 0x4: // MI
			return (cpsr & (1 << 31)) != 0; // N flag
		case 0x5: // PL
			return (cpsr & (1 << 31)) == 0; // N flag
		case 0x6: // VS
			std::cerr << "Status register V flag is not supported" << std::endl;
			return (cpsr & (1 << 28)) != 0; // V flag
		case 0x7: // VC
			return (cpsr & (1 << 28)) == 0; // V flag
		case 0x8: // HI
			return testCondition(0x2) && testCondition(0x1);
		case 0x9: // LS
			return !testCondition(0x8);
		case 0xA: // GE
			return ((cpsr >> N_FLAG) & 1) == ((cpsr >> V_FLAG) & 1);
		case 0xB: // LT
			return !testCondition(0xA);
		case 0xC: // GT
			return testCondition(0x1) && testCondition(0xA);
		case 0xD: // LE
			return !testCondition(0xC);
		case 0xE: // AL
			return true;
		case 0xF: // NV / Reserved
			return false;
		default: // Should not happen
			std::cout << "Unknown condition: " << condition << std::endl;
			return false;
	}
}

bool ARMcpu::executeInstructionThumb(u_int16_t instruction)
{
	InstructionThumb inst;

	PrintHex(instruction, 16);
	std::cout << std::endl;

	if(DecodeInstructionThumb(instruction, inst))
	{
		u_int32_t pcValue = regSet.GetValue(PC);
		bool supported = false;

		PrintInstructionThumb(pcValue, inst);
		std::cout << std::endl;

		switch(inst.type)
		{
			case IT_T_SHIFTED:
			{
				u_int32_t rsv = regSet.GetValue(inst.data.shi.rs);
				u_int32_t result;
				StatusBit carry;
				switch(inst.data.shi.op)
				{
					case 1:
						result = lsr(rsv, inst.data.shi.offset);
						carry = carryShiftRight(rsv, inst.data.shi.offset);	
						break;
					case 2:
						result = asr(rsv, inst.data.shi.offset);
						carry = carryShiftRight(rsv, inst.data.shi.offset);	
						break;
					default:
						result = lsl(rsv, inst.data.shi.offset);
						carry = carryShiftLeft(rsv, inst.data.shi.offset);	
						break;
				}
				regSet.SetValue(inst.data.shi.rd, result);

				updateStatus(result, carry, SB_UNCHANGED);
				pcValue += 2;
				break;
			}

			case IT_T_ADDSUB:
			{
				u_int32_t rsv = regSet.GetValue(inst.data.as.rs);
				if(inst.data.as.op & 2) // Immediate
				{
					if(inst.data.as.op & 1) // SUB
						rsv -= inst.data.as.nn;
					else // ADD
						rsv += inst.data.as.nn;
				}
				else // Register
				{
					u_int32_t rnv = regSet.GetValue(inst.data.as.nn);
					if(inst.data.as.op & 1) // SUB
						rsv -= rnv;
					else
						rsv += rnv;
				}
				regSet.SetValue(inst.data.as.rd, rsv);
				pcValue += 2;
				break;
			}

			case IT_T_IMM:
				aluThumbImm(inst.data.im.op, inst.data.im.rd, inst.data.im.nn);
				pcValue += 2;
				break;

			case IT_T_ALU:
				aluThumbRegister(inst.data.al.op, inst.data.al.rd, inst.data.al.rs);
				pcValue += 2;
				break;

			case IT_T_HIREG:
			{
				u_int32_t rsv = regSet.GetValue(inst.data.hr.rs);
				switch(inst.data.hr.op)
				{
					case 0: // ADD
					{
						u_int32_t rdv = regSet.GetValue(inst.data.hr.rd);
						regSet.SetValue(inst.data.hr.rd, rdv + rsv);
						break;
					}

					case 1: // CMP
					{
						u_int32_t rdv = regSet.GetValue(inst.data.hr.rd);
						u_int32_t result  = rdv - rsv;
						updateStatus(result, result < rdv ? SB_1 : SB_0, SB_UNCHANGED);
						break;
					}

					case 2: // MOV
					{
						regSet.SetValue(inst.data.hr.rd, rsv);
						break;
					}

					case 3:
					{
						pcValue = rsv & 0xFFFFFFFE;
						if(!(rsv & 1)) // Switch to ARM mode
							thumbMode = false;
						if(inst.data.hr.rs == PC)
							pcValue += 4;
						break;	
					}

					default: // Should not happen
						break;
				}

				if(inst.data.hr.op != 3)
					pcValue += 2;

				break;
			}

			case IT_T_LDR_PC:
			{
				u_int32_t address = pcValue + 4 * inst.data.lp.nn + 4;
				u_int32_t value;
				mem.Read(address, value);
				std::cout << "Loading value ";
				PrintHex(value);
				std::cout << std::endl;
				pcValue += 2;
				regSet.SetValue(inst.data.lp.rd, value);
				break;
			}

			case IT_T_MEM_IMM:
			{
				const InstTMemImm *mi = &inst.data.mi;
				u_int32_t rbv = regSet.GetValue(mi->rb);
				u_int32_t address = rbv + mi->nn * ((mi->op & 2) ? 1 : 4);
				
				if(mi->op & 1) // LDR
				{
					if(mi->op & 2) // Byte mode
					{
						u_int32_t value;
						mem.Read(address, value, MA8);
						regSet.SetValue(mi->rd, value);
					}
					else // Word mode
					{
						u_int32_t value;
						mem.Read(address, value);
						regSet.SetValue(mi->rd, value);
					}
				}
				else // STR
				{
					u_int32_t value = regSet.GetValue(mi->rd);
					if(mi->op & 2) // Byte mode
					{
						mem.Write(address, value & 0xFF, MA8);
					}
					else // Word mode
					{
						mem.Write(address, value);
					}
				}
				pcValue += 2;
				break;
			}

			case IT_T_MEM_HALF:
			{
				InstTMemHalf *mh = &inst.data.mh;
				u_int32_t address = regSet.GetValue(mh->rb) + (mh->nn << 1);

				if(mh->op) // LDRH
				{
					u_int32_t value;
					mem.Read(address, value, MA16);
					regSet.SetValue(mh->rd, value);
				}
				else // STRH
				{
					u_int32_t rdv = regSet.GetValue(mh->rd);
					mem.Write(address, rdv, MA16);
				}

				pcValue += 2;
				break;
			}

			case IT_T_MEM_SP:
			{
				InstTMemSP *ms = &inst.data.ms;
				u_int32_t address = regSet.GetValue(SP) + (ms->nn << 2);
				
				if(ms->op) // LDR
				{
					u_int32_t value;
					mem.Read(address, value);
					regSet.SetValue(ms->rd, value);
				}
				else // STR
				{
					u_int32_t rdv = regSet.GetValue(ms->rd);
					mem.Write(address, rdv);
				}

				pcValue += 2;
				break;
			}
			
			case IT_T_RELATIVE:
			{
				u_int32_t value = inst.data.r.nn;
				if(inst.data.r.sp) // SP
					value += regSet.GetValue(SP);
				else // PC
					value += (regSet.GetValue(PC) + 4) & ~2;
				regSet.SetValue(inst.data.r.rd, value);
				pcValue += 2;
				break;
			}
			
			case IT_T_ADDSP:
			{
				u_int32_t spv = regSet.GetValue(SP);
				spv += 4 * inst.data.asp.offset;
				regSet.SetValue(SP, spv);
				pcValue += 2;
				break;
			}

			case IT_T_PUSHPOP:
			{
				InstTPushPop *pp = &inst.data.pp;
				u_int32_t spv = regSet.GetValue(SP);
				u_int32_t value;

				if(pp->op) // POP
				{
					printRegisters(regSet, pp->rlist);
					for(unsigned i = 8; i;)
					{
						--i;
						if((pp->rlist >> i) & 1) // Register must be popped
						{
							std::cout << "Popping value @";
							PrintHex(spv);
							std::cout << std::endl;
							mem.Read(spv, value);
							regSet.SetValue(i, value);
							spv += 4;
						}
					}
					if(pp->pclr) // PC must me popped
					{
						mem.Read(spv, value);
						std::cout << "POP PC = ";
						PrintHex(value);
						std::cout << std::endl;
						pcValue = value & 0xFFFFFFFE;
						spv += 4;
				
						/* Do not update thumb mode?	
						if(!(value & 1))
							thumbMode = false;
						*/
					}
					for(unsigned int i = 0; i < 16; ++i)
						std::cout << '-';
					std::cout << std::endl;
					printRegisters(regSet, pp->rlist);
				}
				else // PUSH
				{
					printRegisters(regSet, pp->rlist);
					if(pp->pclr)
					{
						std::cout << "lr: ";
						PrintHex(regSet.GetValue(LR));
						std::cout << std::endl;
					}
					
					if(pp->pclr) // LR must me pushed
					{
						spv -= 4;
						std::cout << "PUSH @";
					   	PrintHex(spv);
						std::cout << " LR = ";
						PrintHex(regSet.GetValue(LR));
						std::cout << std::endl;
						mem.Write(spv, regSet.GetValue(LR));
					}
					for(unsigned i = 0; i < 8; ++i)
					{
						if((pp->rlist >> i) & 1) // Register must be pushed
						{
							spv -= 4;
							std::cout << "Pushing value @";
							PrintHex(spv);
							std::cout << std::endl;
							mem.Write(spv, regSet.GetValue(i));
						}
					}
				}
				regSet.SetValue(SP, spv);
				if(!pp->op || !pp->pclr)
					pcValue += 2;
				break;
			}

			case IT_T_MULTIPLE:
			{
				u_int32_t rbv = regSet.GetValue(inst.data.m.rb);
				PrintRegister(inst.data.m.rb);
				std::cout << ": ";
				PrintHex(rbv);
				std::cout << std::endl;
				if(inst.data.m.op) // LDM
				{
					for(unsigned int i = 0; i < 8; ++i)
						if((inst.data.m.rlist >> i) & 1)
						{
							u_int32_t value;
							mem.Read(rbv, value);
							regSet.SetValue(i, value);
							rbv += 4;
						}
				}
				else // STM
				{
					for(unsigned int i = 0; i < 8; ++i)
						if((inst.data.m.rlist >> i) & 1)
						{
							u_int32_t value = regSet.GetValue(i);
							mem.Write(rbv, value);
							rbv += 4;
						}
				}
				
				regSet.SetValue(inst.data.m.rb, rbv);
				pcValue += 2;
				break;
			}

			case IT_T_BRANCH_COND:
				if(testCondition(inst.data.bc.cond))
					pcValue += inst.data.bc.offset;
				else
					pcValue += 2;
				break;

			case IT_T_SVC:
			{
				pcValue += 2;
				break;
				regSet.SetMode(RS_SUPERVISOR);
				std::cout << "Entering supervisor mode" << std::endl;
				/*
				// Software interrupt address = BASE + 0x08
				mem.Read(baseAddress + (inst.data.svc.op << 2) + 8, pcValue);
				//pcValue += baseAddress;
				std::cout << "@";
				PrintHex(baseAddress + (inst.data.svc.op << 2) + 8);
				std::cout << ": ";
				PrintHex(pcValue);
				std::cout << std::endl;
				*/

				thumbMode = false;
				pcValue = baseAddress + 0x08;

				break;
			}

			case IT_T_BRANCH:
				pcValue += inst.data.b.offset;
				break;
	
			case IT_T_BRANCH_LINK:
				if(inst.data.bl.lr)
				{
					regSet.SetValue(LR, (pcValue + 4 + (inst.data.bl.nn << 12)) | 1);
					pcValue += 2;
				}
				else
				{
					u_int32_t lrv = regSet.GetValue(LR);
					if(inst.data.bl.x) // BLX
					{
						regSet.SetValue(LR, pcValue + 2);
						thumbMode = false;
					}
					else
						regSet.SetValue(LR, (pcValue + 2) | 1);
					pcValue += Unsigned2Signed(lrv + (inst.data.bl.nn << 1) - pcValue, 22);
					pcValue &= 0xFFFFFFFE;
				}
				break;

			default:
				std::cerr << "Instruction not supported yet" << std::endl;
				pcValue += 2;
				supported = false;
				break;
		}
		
		regSet.SetValue(PC, pcValue);
		return !supported;
	}
	else
		std::cerr << "Unknown thumb instruction" << std::endl;
	
	return false;
}

