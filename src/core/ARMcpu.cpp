#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>

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

/*
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
*/

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
		/*
		u_int32_t pcvalue = regSet.GetValue(PC);
		if(pcvalue == 0x18 // BIOS interrupt routine
		|| pcvalue == 0x08043980)
			lock = true;
		*/
		runStep();

		if(lock)
			getchar();
	}
}

void ARMcpu::runStep(void)
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
		executeInstructionThumb(inst);
		std::cout << std::endl;
	}

	onClock();
}

void ARMcpu::onClock(void)
{
	++cycles;
}

bool ARMcpu::interruptsEnabled(void)
{
	return (regSet.GetValue(CPSR) & (1 << 7)) == 0;
}

void ARMcpu::executeInstruction(u_int32_t instruction)
{
	Instruction inst;

	PrintHex(instruction);
	std::cout << std::endl;

	if(DecodeInstruction(instruction, inst))
	{
		u_int32_t pcValue = regSet.GetValue(PC);
		DataWrapper wrapper(&pcValue);

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

					if(dp1->rm == PC)
						rmv += 8;

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
					alu(dp1->op, dp1->s, dp1->rd, dp1->rn, op2, scarry, wrapper);
				}
				break;
			case IT_MEM_SPECIAL:
				if(testCondition(inst.data.ms.cond))
				{
					InstMemSpecial *ms = &inst.data.ms;
					u_int32_t rdv = regSet.GetValue(ms->rd);
					u_int32_t rnv = regSet.GetValue(ms->rn);
					int32_t offset;
					u_int32_t tmp;
					
					//u_int32_t op2;
					//StatusBit scarry;
					
					std::cout << "MemSpecial" << std::endl;
					
					if(ms->rd == PC)
						rdv += 8;
					
					if(ms->rn == PC)
						rnv += 12;

					if(ms->i) // immediate
						offset = (ms->u ? ms->offset : -ms->offset);
					else // register
					{
						u_int32_t rmv = regSet.GetValue(ms->rm);
						offset = (ms->u ? rmv : -rmv);
					}

					if(ms->p) // pre
						rnv += offset;

					if(ms->l) // ldrXX
					{
						switch(ms->op)
						{
							case 1: // ldrh
								mem.Read(rnv, tmp, MA32);
								tmp &= 0xFFFF;
								regSet.SetValue(ms->rd, tmp, &wrapper);
								break;
							case 2: // ldrsb
								mem.Read(rnv, tmp, MA32);
								tmp &= 0xFF;
								tmp |= ((tmp >> 7) & 1) * 0xFFFFFF00;
								regSet.SetValue(ms->rd, tmp, &wrapper);
								break;
							case 3: // ldrsh
								mem.Read(rnv, tmp, MA32);
								tmp &= 0xFFFF;
								tmp |= ((tmp >> 15) & 1) * 0xFFFF0000;
								regSet.SetValue(ms->rd, tmp, &wrapper);
								break;
							default:
								std::cerr << "Reserved opcode" << std::endl;
						}
					}
					else
					{
						switch(ms->op)
						{
							case 1: // strh
								mem.Write(rnv, regSet.GetValue(ms->rd) & 0x0000FFFF);
								break;
							case 2: // ldrd
							{
								u_int32_t tmp1;
								mem.Read(rnv, tmp, MA32);
								mem.Read(rnv + 4, tmp1, MA32);
								regSet.SetValue(ms->rd, tmp, &wrapper);
								regSet.SetValue(ms->rd + 1, tmp1, &wrapper);
								break;
							}
							case 3: // strd
								mem.Write(rnv, regSet.GetValue(ms->rd));
								mem.Write(rnv + 4, regSet.GetValue(ms->rd + 1));
								break;
							default:
								std::cerr << "Reserved opcode" << std::endl;
						}
					}

					if(ms->p == 0 || ms->w) // post and/or write-back
					{
						u_int32_t tmp = regSet.GetValue(ms->rn) + offset;
						regSet.SetValue(ms->rn, tmp, &wrapper);
					}

					/* ???
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
					alu(dp2->op, dp2->s, dp2->rd, dp2->rn, op2, scarry, wrapper);
					*/
				}
				break;

			case IT_DATA_PROC3:
				if(testCondition(inst.data.dp3.cond))
				{
					InstDataProc3 *dp3 = &inst.data.dp3;
					u_int32_t op2 = ror(dp3->immediate, dp3->shift);
					StatusBit scarry = carryShiftRight(dp3->immediate, dp3->shift);
					alu(dp3->op, dp3->s, dp3->rd, dp3->rn, op2, scarry, wrapper);
				}
				break;

			case IT_PSR_REG:
				if(testCondition(inst.data.psrr.cond))
				{
					InstPSRReg *psrr = &inst.data.psrr;
					ARMRegister reg = psrr->p ? SPSR : CPSR;
					u_int32_t value = regSet.GetValue(reg);

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
						regSet.SetValue(reg, value, 0);
					}
					else // MRS
					{
						regSet.SetValue(psrr->rd, value, 0);
					}
				}
				break;
			
			case IT_BRANCH:
				if(testCondition(inst.data.b.cond))
				{
					if(inst.data.b.l) // link
						regSet.SetValue(LR, pcValue + 4, 0);
					wrapper.Set(pcValue + inst.data.b.offset);
				}
				break;

			case IT_BRANCH_EX:
				if(testCondition(inst.data.bx.cond))
				{
					std::cout << "REGISTER: ";
					PrintRegister(inst.data.bx.rn);
					std::cout << " (";
					u_int32_t rnv = regSet.GetValue(inst.data.bx.rn);
					PrintHex(rnv);
					std::cout << ");" << std::endl;
					if(inst.data.bx.l) // link
						regSet.SetValue(LR, pcValue + 4, 0);

					wrapper.Set(rnv & 0xFFFFFFFE);
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
						regSet.SetValue(inst.data.ti9.rd, value, &wrapper);
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
						regSet.SetValue(inst.data.ti9.rn, address, &wrapper);
				}
				break;

			case IT_BLOCK_TRANS:
			{
				const InstBlockTrans *bt = &inst.data.bt;
				if(testCondition(bt->cond))
				{
					u_int32_t rnv = ARMcpu::loadstore(bt->rn, bt->rlist, false, true, bt->l > 0, bt->u > 0, bt->p == 0, wrapper);
					if(bt->w) // Write-back
						regSet.SetValue(bt->rn, rnv, &wrapper);
				}
				break;
			}

			default:
				std::cerr << "Instruction not supported yet" << std::endl;
				break;
		}

		if(!wrapper.IsWritten())
			pcValue += 4;

		regSet.SetValue(PC, pcValue, 0);
	}
	else
	{
		std::cerr << "Unknown instruction..." << std::endl;
		exit(0);
	}
}

void ARMcpu::alu(u_int8_t op, u_int8_t s, u_int8_t rd, u_int8_t rn, u_int32_t op2, StatusBit shiftCarry, DataWrapper &pcValue)
{
	u_int32_t rnv = 0;
	u_int32_t result;

	StatusBit carry = SB_UNCHANGED;
	StatusBit overflow = SB_UNCHANGED;

	if(op != 0xD && op != 0xF)
	{
		rnv = regSet.GetValue(rn);
		if(rn == PC)
		{
			std::cout << "PC DETECTED"<<std::endl;
			rnv += 8;
		}
	}

	switch(op)
	{
		case 0x0: // AND
		{
			result = rnv & op2;
			carry = shiftCarry;
			break;
		}
		case 0x1: // EOR
		{
			result = rnv ^ op2;
			carry = shiftCarry;
			break;
		}
		case 0x2: // SUB
		{
			result = rnv - op2;
			if(s)
				carry = result > rnv ? SB_1 : SB_0;
			break;
		}
		case 0x3: // RSB
		{
			result = op2 - rnv;
			if(s)
				carry = result > rnv ? SB_1 : SB_0;
			break;
		}
		case 0x4: // ADD
		{
			result = rnv + op2;
			if(s)
				carry = result < rnv ? SB_1 : SB_0;
			break;
		}
		case 0x5: // ADC
		{
			u_int32_t cpsr = regSet.GetValue(CPSR);
			result = rnv + op2 + ((cpsr >> 29) & 1);
			if(s)
				carry = result < rnv ? SB_1 : SB_0;
			break;
		}
		case 0x6: // SBC
		{
			u_int32_t cpsr = regSet.GetValue(CPSR);
			result = rnv - op2 - 1 + ((cpsr >> 29) & 1);
			if(s)
				carry = result > rnv ? SB_1 : SB_0;
			break;
		}
		case 0x7: // RSC
		{
			u_int32_t cpsr = regSet.GetValue(CPSR);
			result = op2 - rnv - 1 + ((cpsr >> 29) & 1);
			if(s)
				carry = result > rnv ? SB_1 : SB_0;
			break;
		}
		case 0x8: // TST
		{
			result = rnv & op2;
			carry = shiftCarry;
			break;
		}
		case 0x9: // TEQ
		{
			result = rnv ^ op2;
			carry = shiftCarry;
			break;
		}
		case 0xA: // CMP
		{
			result = rnv - op2;
			if(s)
				carry = result > rnv ? SB_1 : SB_0;
			break;
		}
		case 0xB: // CMN
		{
			result = rnv + op2;
			if(s)
				carry = result < rnv ? SB_1 : SB_0;
			break;
		}
		case 0xC: // ORR
		{
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
			result = rnv & (~op2);
			carry = shiftCarry;
			break;
		}
		case 0xF: // MVN
			result = ~op2;
			carry = shiftCarry;
			break;
		default:
			result = 0;
			break;
	}
	
	if(op <= 0x7 || (op >= 0xC && op <= 0xF))
	{
		regSet.SetValue(rd, result, &pcValue);
		if(s)
			updateStatus(result, carry, overflow);
	}
	else
		updateStatus(result, carry, overflow);
}

void ARMcpu::aluThumbRegister(u_int8_t op, u_int8_t rd, u_int8_t rs, DataWrapper &pcValue)
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
			result = 0;
			break;
	}
	if(op != 0x8 && op != 0xA && op != 0xB)
		regSet.SetValue(rd, result, &pcValue);

	updateStatus(result, carry, overflow);
}

void ARMcpu::aluThumbImm(u_int8_t op, u_int8_t rd, u_int8_t nn, DataWrapper &pcValue)
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
			result = 0;

			break;
	}

	if(op != 1) // Not CMP
		regSet.SetValue(rd, result, &pcValue);
	
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

	regSet.SetValue(CPSR, cpsr, 0);
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

void ARMcpu::executeInstructionThumb(u_int16_t instruction)
{
	InstructionThumb inst;

	PrintHex(instruction, 16);
	std::cout << std::endl;

	if(DecodeInstructionThumb(instruction, inst))
	{
		u_int32_t pcValue = regSet.GetValue(PC);
		DataWrapper wrapper(&pcValue);

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
				regSet.SetValue(inst.data.shi.rd, result, &wrapper);

				updateStatus(result, carry, SB_UNCHANGED);
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
				regSet.SetValue(inst.data.as.rd, rsv, &wrapper);
				break;
			}

			case IT_T_IMM:
				aluThumbImm(inst.data.im.op, inst.data.im.rd, inst.data.im.nn, wrapper);
				break;

			case IT_T_ALU:
				aluThumbRegister(inst.data.al.op, inst.data.al.rd, inst.data.al.rs, wrapper);
				break;

			case IT_T_HIREG:
			{
				u_int32_t rsv = regSet.GetValue(inst.data.hr.rs);
				switch(inst.data.hr.op)
				{
					case 0: // ADD
					{
						u_int32_t rdv = regSet.GetValue(inst.data.hr.rd);
						regSet.SetValue(inst.data.hr.rd, rdv + rsv, &wrapper);
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
						regSet.SetValue(inst.data.hr.rd, rsv, &wrapper);
						break;
					}

					case 3:
					{
						wrapper.Set(rsv & 0xFFFFFFFE);
						if(!(rsv & 1)) // Switch to ARM mode
							thumbMode = false;
						if(inst.data.hr.rs == PC)
							wrapper.Set(pcValue + 4);
						break;	
					}

					default: // Should not happen
						break;
				}
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
				regSet.SetValue(inst.data.lp.rd, value, &wrapper);
				break;
			}

			case IT_T_MEM_SPECIAL:
			{
				const InstTMemSpecial *s = &inst.data.s;
				u_int32_t rdv = regSet.GetValue(s->rd);
				u_int32_t address = regSet.GetValue(s->rb) + regSet.GetValue(s->ro);
				u_int32_t tmp;

				switch(s->op)
				{
					case 0: // strh
						mem.Write(address, rdv & 0xFFFF);
						break;
					case 1: // ldrsb
						mem.Read(address, tmp, MA32);
						tmp &= 0xFF;
						tmp |= ((tmp >> 7) & 1) * 0xFFFFFF00;
						regSet.SetValue(s->rd, tmp, &wrapper);
						break;
					case 2: // ldrh
						mem.Read(address, tmp, MA32);
						regSet.SetValue(s->rd, tmp & 0xFFFF, &wrapper);
						break;
					default: // ldrsh
						mem.Read(address, tmp, MA32);
						tmp &= 0xFFFF;
						tmp |= ((tmp >> 15) & 1) * 0xFFFF0000;
						regSet.SetValue(s->rd, tmp, &wrapper);
						break;
				}
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
						regSet.SetValue(mi->rd, value, &wrapper);
					}
					else // Word mode
					{
						u_int32_t value;
						mem.Read(address, value);
						regSet.SetValue(mi->rd, value, &wrapper);
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
					regSet.SetValue(mh->rd, value, &wrapper);
				}
				else // STRH
				{
					u_int32_t rdv = regSet.GetValue(mh->rd);
					mem.Write(address, rdv, MA16);
				}
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
					regSet.SetValue(ms->rd, value, &wrapper);
				}
				else // STR
				{
					u_int32_t rdv = regSet.GetValue(ms->rd);
					mem.Write(address, rdv);
				}
				break;
			}
			
			case IT_T_RELATIVE:
			{
				u_int32_t value = inst.data.r.nn;
				if(inst.data.r.sp) // SP
					value += regSet.GetValue(SP);
				else // PC
					value += (regSet.GetValue(PC) + 4) & ~2;
				regSet.SetValue(inst.data.r.rd, value, &wrapper);
				break;
			}
			
			case IT_T_ADDSP:
			{
				u_int32_t spv = regSet.GetValue(SP);
				spv += 4 * inst.data.asp.offset;
				regSet.SetValue(SP, spv, 0);
				break;
			}

			case IT_T_PUSHPOP:
			{
				const InstTPushPop *pp = &inst.data.pp;
				u_int32_t spv = regSet.GetValue(SP);
	
				spv = loadstore(SP, pp->rlist, pp->pclr > 0, false, pp->op > 0, pp->op > 0, pp->op > 0, wrapper);
				regSet.SetValue(SP, spv, 0);
				break;
			}

			case IT_T_MULTIPLE:
			{
				u_int32_t rbv = loadstore(inst.data.m.rb, inst.data.m.rlist, false, false, inst.data.m.op > 0, true, true, wrapper);
				regSet.SetValue(inst.data.m.rb, rbv, &wrapper);
				break;
			}

			case IT_T_BRANCH_COND:
				if(testCondition(inst.data.bc.cond))
					wrapper.Set(pcValue + inst.data.bc.offset);
				break;

			case IT_T_SVC:
			{
				biosCall(inst.data.svc.op);
				break;
			}

			case IT_T_BRANCH:
				wrapper.Set(pcValue + inst.data.b.offset);
				break;
	
			case IT_T_BRANCH_LINK:
				if(inst.data.bl.lr)
				{
					regSet.SetValue(LR, (pcValue + 4 + (inst.data.bl.nn << 12)) | 1, 0);
				}
				else
				{
					u_int32_t lrv = regSet.GetValue(LR);
					if(inst.data.bl.x) // BLX
					{
						regSet.SetValue(LR, pcValue + 2, 0);
						thumbMode = false;
					}
					else
						regSet.SetValue(LR, (pcValue + 2) | 1, 0);
					wrapper.Set(pcValue + Unsigned2Signed(lrv + (inst.data.bl.nn << 1) - pcValue, 22));
					wrapper.Set(pcValue & 0xFFFFFFFE);
				}
				break;

			default:
				std::cerr << "Instruction not supported yet" << std::endl;
				break;
		}

		if(!wrapper.IsWritten())
			pcValue += 2;

		regSet.SetValue(PC, pcValue, 0);
	}
	else
		std::cerr << "Unknown thumb instruction" << std::endl;
}

u_int32_t ARMcpu::loadstore(unsigned int reg, u_int16_t registers, bool pclr, bool r16, bool load, bool increment, bool after, DataWrapper &pcwrapper)
{
	unsigned int maxregistercount = (r16 ? 16 : 8);
	const u_int32_t delta = 4;
	u_int32_t value;
	u_int32_t finalAddress;
	u_int32_t lowestAddress = regSet.GetValue(reg);
	std::vector<unsigned int> targetRegisters;

	for(unsigned int i = 0; i < maxregistercount; ++i)
		if((registers >> i) & 1)
			targetRegisters.push_back(i);
	if(pclr)
		targetRegisters.push_back((unsigned int)(load ? PC : LR));

	if(increment)
	{
		finalAddress = lowestAddress + targetRegisters.size() * 4;
	}
	else
	{
		lowestAddress -= (targetRegisters.size()+1) * 4;
		finalAddress = lowestAddress + 4;
	}

	std::cout << "PUSHPOP" << std::endl;

	if(!after)
	{
		lowestAddress += delta;
		//finalAddress += 4;
	}

	for(std::vector<unsigned int>::const_iterator it = targetRegisters.begin();
	    it != targetRegisters.end();
		++it)
	{
		if(load)
		{
			mem.Read(lowestAddress, value);
			regSet.SetValue(*it, value, &pcwrapper);

			PrintRegister(*it);
			std::cout << " <- ";
			PrintHex(value);
			std::cout << " (";
			PrintHex(lowestAddress);
			std::cout << ')';
			std::cout << std::endl;
		}
		else
		{
			value = regSet.GetValue(*it);
			mem.Write(lowestAddress, value);

			PrintRegister(*it);
			std::cout << " (";
			PrintHex(value);
			std::cout << ") -> @";
			PrintHex(lowestAddress);
			std::cout << std::endl;
		}

		lowestAddress += delta;
	}
	std::cout << "Final address: ";
	PrintHex(finalAddress);
	std::cout << std::endl;

	return finalAddress;
}

void ARMcpu::biosCall(u_int8_t op)
{
	switch(op)
	{
		//case 0x4: // IntrWait

		//	break;

		case 0x5: // VBlankIntrWait
			mem.Write(0x4000208, 0); // Enable IME
			regSet.SetValue(R0, 1, 0);
			regSet.SetValue(R1, 1, 0);
			biosCall(0x4);
			break;

		case 0xb: // CpuSet
		{
			u_int32_t src = regSet.GetValue(R0);
			u_int32_t dst = regSet.GetValue(R1);
			u_int32_t params = regSet.GetValue(R2);
			u_int32_t count = params & 0x1FFFFF;
			bool word = (params & (1 << 26)) != 0;
			MemoryAccess memAccess = (word ? MA32 : MA16);

			// Debug
			std::cout << "mem";
			if(params & (1<<24))
				std::cout << "fill";
			else
				std::cout << "cpy";
			if(!word)
				std::cout << "16";
			std::cout << std::endl << "  src   = ";
			PrintHex(src);
			std::cout << std::endl << "  dst   = ";
			PrintHex(dst);
			std::cout << std::endl << "  count = " << count;
			std::cout << std::endl;
			
			// Execution
			if(word)
			{
				src &= 0xFFFFFFFC;
				dst &= 0xFFFFFFFC;
			}
			else
			{
				src &= 0xFFFFFFFE;
				dst &= 0xFFFFFFFE;
			}

			for(u_int32_t i = 0; i < count; ++i)
			{
				u_int32_t tmp;
				mem.Read(src, tmp, memAccess);
				mem.Write(dst, tmp, memAccess);

				if(!(params & (1<<24))) // cpy
					src += (word ? 4 : 2);
				dst += (word ? 4 : 2);
			}
			break;
		}

		default:
			std::cerr << "Unknown SVC command " << (unsigned int) op << std::endl;
	}
}

