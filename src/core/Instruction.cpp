#include <iostream>

#include "Instruction.h"
#include "ARMcpu.h"
#include "utils.h"

bool DecodeInstruction(u_int32_t code, Instruction &instruction)
{
	u_int8_t id = (code >> 25) & 7;
	instruction.type = IT_UNKNOWN;

	std::cout << "op = " << (int)id << std::endl;	
	switch(id)
	{
		case 0:
			if(((code >> 4) & 0xFFFFFD) == 0x12FFF1) // BX, BLX_reg
			{
				instruction.type = IT_BRANCH_EX;
				instruction.data.bx.cond = code >> 28;
				instruction.data.bx.l = (code >> 5) & 1;
				instruction.data.bx.rn = code & 0xF;
			}
			else
			{
				u_int8_t op = (code >> 21) & 0xF;
				u_int8_t s = (code >> 20) & 1;

				if(op >= 0x8 && op <= 0xB && !s) // PSR
				{
					instruction.type = IT_PSR_REG;
					instruction.data.psrr.cond = code >> 28;
					instruction.data.psrr.p = (code >> 22) & 1;
					instruction.data.psrr.l = (code >> 21) & 1;
					instruction.data.psrr.field = (code >> 16) & 0xF;
					instruction.data.psrr.rd = (code >> 12) & 0xF;
					instruction.data.psrr.rm = code & 0xF;
				}
				else if((code & (1 << 4)) == 0) // Probably DP1
				{
					instruction.type = IT_DATA_PROC1;
					instruction.data.dp1.cond = code >> 28;
					instruction.data.dp1.op = op;
					instruction.data.dp1.s = s;
					instruction.data.dp1.rn = (code >> 16) & 0xF;
					instruction.data.dp1.rd = (code >> 12) & 0xF;
					instruction.data.dp1.shift = (code >> 7) & 0x1F;
					instruction.data.dp1.typ = (code >> 5) & 0x3;
					instruction.data.dp1.rm = code & 0xF;
				}
				else // Probably DP2
				{
					instruction.type = IT_DATA_PROC2;
					instruction.data.dp2.cond = code >> 28;
					instruction.data.dp2.op = op;
					instruction.data.dp2.s = s;
					instruction.data.dp2.rn = (code >> 16) & 0xF;
					instruction.data.dp2.rd = (code >> 12) & 0xF;
					instruction.data.dp2.rs = (code >> 8) & 0xF;
					instruction.data.dp2.typ = (code >> 5) & 0x3;
					instruction.data.dp2.rm = code & 0xF;
				}
			}
			return true;

		case 1:
		{
			u_int8_t op = (code >> 21) & 0xF;
			u_int8_t s = (code >> 20) & 1;

			if(op >= 0x8 && op <= 0xB && !s) // PSR
			{
				// TODO
			}
			else // DP3
			{
				instruction.type = IT_DATA_PROC3;
				instruction.data.dp3.cond = code >> 28;
				instruction.data.dp3.op = (code >> 21) & 0xF;
				instruction.data.dp3.s = (code >> 20) & 1;
				instruction.data.dp3.rn = (code >> 16) & 0xF;
				instruction.data.dp3.rd = (code >> 12) & 0xF;
				instruction.data.dp3.shift = 2 * ((code >> 8) & 0xF);
				instruction.data.dp3.immediate = code & 0xFF;
			}
			return true;
		}

		case 2:
			// LDR, STR, PLD
			instruction.type = IT_TRANS_IMM9;
			instruction.data.ti9.cond = code >> 28;
			instruction.data.ti9.p = (code >> 24) & 1;
			instruction.data.ti9.u = (code >> 23) & 1;
			instruction.data.ti9.b = (code >> 22) & 1;
			instruction.data.ti9.w = (code >> 21) & 1;
			instruction.data.ti9.l = (code >> 20) & 1;
			instruction.data.ti9.rn = (code >> 16) & 0xF;
			instruction.data.ti9.rd = (code >> 12) & 0xF;
			instruction.data.ti9.offset = code & 0xFFF;
			return true;

		case 3:
			break;

		case 4:
			instruction.type = IT_BLOCK_TRANS;
			instruction.data.bt.cond = code >> 28;
			instruction.data.bt.p = (code >> 24) & 1;
			instruction.data.bt.u = (code >> 23) & 1;
			instruction.data.bt.s = (code >> 22) & 1;
			instruction.data.bt.w = (code >> 21) & 1;
			instruction.data.bt.l = (code >> 20) & 1;
			instruction.data.bt.rn = (code >> 16) & 0xF;
			instruction.data.bt.rlist = code & 0xFFFF;
			return true;

		case 5:
		{
			int32_t offset = (code & ((1 << 24) - 1));

			// B,BL,BLX
			instruction.type = IT_BRANCH;
			instruction.data.b.cond = code >> 28;
			instruction.data.b.l = (code >> 24) & 1;
			instruction.data.b.offset = 8 + 4 * Unsigned2Signed(offset, 23);
			return true;
		}

		case 6:
			break;

		case 7:
			break;

		default: // Should not happen
			break;
	}
	return false;
}

void PrintInstruction(const Instruction &instruction)
{
	const char *aluInst[] = {
		"and",
		"eor",
		"sub",
		"rsb",
		"add",
		"adc",
		"sbc",
		"rsc",
		"tst",
		"teq",
		"cmp",
		"cmn",
		"orr",
		"mov",
		"bic",
		"mvn"
	};

	const char *shiftTypes[] = {
		"lsl",
		"lsr",
		"asr",
		"ror"
	};

	const char *psr[] = {
		"cpsr",
		"spsr"
	};

	switch(instruction.type)
	{
		case IT_DATA_PROC1:
		{
			const InstDataProc1 *dp1 = &instruction.data.dp1;
			std::cout << aluInst[dp1->op];
			PrintCondition(dp1->cond);
			if(dp1->s)
				std::cout << 's';
			std::cout << ' ';
			if(dp1->op <= 0x7 || dp1->op >= 0xC)
			{
				PrintRegister(dp1->rd);
				std::cout << ", ";
			}
			if(dp1->op != 0xD && dp1->op != 0xF)
			{
				PrintRegister(dp1->rn);
				std::cout << ", ";
			}

			PrintRegister(dp1->rm);
			std::cout << ' ' << shiftTypes[dp1->typ] << " #";
			std::cout << (unsigned int) dp1->shift;
			return;
		}

		case IT_DATA_PROC3:
		{
			const InstDataProc3 *dp3 = &instruction.data.dp3;
			std::cout << aluInst[dp3->op];
			PrintCondition(dp3->cond);
			if(dp3->s)
				std::cout << 's';
			std::cout << ' ';
			if(dp3->op <= 0x7 || dp3->op >= 0xC)
			{
				PrintRegister(dp3->rd);
				std::cout << ", ";
			}
			if(dp3->op != 0xD && dp3->op != 0xF)
			{
				PrintRegister(dp3->rn);
				std::cout << ", ";
			}

			std::cout << "#" << (unsigned int)dp3->immediate;
			std::cout << " ROR " << (int)(dp3->shift);	
			return;
		}

		case IT_PSR_REG:
		{
			const InstPSRReg *psrr = &instruction.data.psrr;
			const char *sr = psr[psrr->p];

			if(psrr->l) // MSR
			{
				const char flags[] = {'c', 'x', 's', 'f'};
				std::cout << "msr";
				PrintCondition(psrr->cond);
				std::cout << ' ' << sr << '_';
				// Print flags
				for(unsigned int i = 4; i;)
					if((psrr->field >> (--i)) & 1)
						std::cout << flags[i];

				std::cout << ", ";
				PrintRegister(psrr->rm);
			}
			else // MRS
			{
				std::cout << "mrs";
				PrintCondition(psrr->cond);
				std::cout << ' ';
				PrintRegister(psrr->rd);
				std::cout << ", " << sr;
			}
			return;
		}

		case IT_BRANCH:
			std::cout << "b";
			if(instruction.data.b.l)
				std::cout << "l";
			std::cout << " #" << instruction.data.b.offset;
			return;

		case IT_TRANS_IMM9:
		{
			if(instruction.data.ti9.l)
				std::cout << "ldr";
			else
				std::cout << "str";
			std::cout << ' ';
			PrintCondition(instruction.data.ti9.cond);
			if(instruction.data.ti9.b)
				std::cout << ".b";
			PrintRegister(instruction.data.ti9.rd);
			std::cout << ", ";
			if(!instruction.data.ti9.u)
				std::cout << '-';
			
			std::cout << (unsigned int)instruction.data.ti9.offset << '(';
			PrintRegister(instruction.data.ti9.rn);
			std::cout << ')';
			return;
		}

		case IT_BLOCK_TRANS:
		{
			bool first = true;

			if(!instruction.data.bt.l && !instruction.data.bt.u && instruction.data.bt.p)
				std::cout << "push";
			else if(instruction.data.bt.l && instruction.data.bt.u && !instruction.data.bt.p)
				std::cout << "pop";
			else
			{
				std::cout << (instruction.data.bt.l ? "ldm" : "stm");
				std::cout << (instruction.data.bt.u ? 'i' : 'd');
				std::cout << (instruction.data.bt.p ? 'b' : 'a');
				PrintCondition(instruction.data.bt.cond);
				std::cout << ' ';
				PrintRegister(instruction.data.bt.rn);
				if(instruction.data.bt.w)
					std::cout << '!';
				std::cout << ',';
			}

			std::cout << " {";
			for(unsigned int i = 0; i < 16; ++i)
				if((instruction.data.bt.rlist >> i) & 1)
				{
					if(!first)
						std::cout << ',';
					PrintRegister(i);
					first = false;
				}
			std::cout << '}';
			if(instruction.data.bt.s)
				std::cout << '^';
			return;
		}

		default:
			break;
	}
	std::cout << "I was too lazy to implement disassembly for this instruction" ;
}

