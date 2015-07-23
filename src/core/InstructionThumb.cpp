#include <cstdlib>
#include <iostream>

#include "InstructionThumb.h"
#include "utils.h"

static u_int32_t blAddress = 0;

bool DecodeInstructionThumb(u_int16_t code, InstructionThumb &instruction)
{
	u_int8_t id = code >> 13;

	std::cout << "op = " << (int)id << std::endl;

	switch(id)
	{
		case 0:
		{
			u_int8_t op = (code >> 11) & 0x3;
			if(op == 0x3) // ADD/SUB
			{
				instruction.type = IT_T_ADDSUB;
				instruction.data.as.op = (code >> 9) & 0x3;
				instruction.data.as.nn = (code >> 6) & 0x3;
				instruction.data.as.rs = (code >> 3) & 0x7;
				instruction.data.as.rd = code & 0x7;
			}
			else // shifted MOV
			{
				instruction.type = IT_T_SHIFTED;
				instruction.data.shi.op = op;
				instruction.data.shi.offset = (code >> 6) & 0x1F;
				instruction.data.shi.rs = (code >> 3) & 0x7;
				instruction.data.shi.rd = code & 0x7;
			}
			return true;
		}

		case 1:
			instruction.type = IT_T_IMM;
			instruction.data.im.op = (code >> 11) & 0x3;
			instruction.data.im.rd = (code >> 8) & 0x7;
			instruction.data.im.nn = code & 0xFF;
			return true;

		case 2:
			if(((code >> 11) & 0x3) == 1) // LDR PC
			{
				instruction.type = IT_T_LDR_PC;
				instruction.data.lp.rd = (code >> 8) & 0x7;
				instruction.data.lp.nn = code & 0xFF;
				return true;
			}
			else if(((code >> 10) & 0x7) == 1) // HiReg/BX
			{
				instruction.type = IT_T_HIREG;
				instruction.data.hr.op = (code >> 8) & 0x3;
				instruction.data.hr.rs = ((code & (1 << 6)) >> 3) | ((code >> 3) & 0x7);
				instruction.data.hr.rd = ((code & (1 << 7)) >> 4) | (code & 0x7);
				return true;
			}
			else if(((code >> 10) & 0x7) == 0) // AluOp
			{
				instruction.type = IT_T_ALU;
				instruction.data.al.op = (code >> 6) & 0xF;
				instruction.data.al.rs = (code >> 3) & 0x7;
				instruction.data.al.rd = code & 0x7;
				return true;
			}
			break;

		case 3:
			instruction.type = IT_T_MEM_IMM;
			instruction.data.mi.op = (code >> 11) & 0x3;
			instruction.data.mi.nn = (code >> 6) & 0x1F;
			instruction.data.mi.rb = (code >> 3) & 0x7;
			instruction.data.mi.rd = code & 0x7;
			return true;

		case 4:
			if(code & (1 << 12)) // Load/Store SP-relative
			{
				instruction.type = IT_T_MEM_SP;
				instruction.data.ms.op = (code >> 11) & 1;
				instruction.data.ms.rd = (code >> 8) & 0x7;
				instruction.data.ms.nn = code & 0xFF;
			}
			else // Load/Store halfword
			{
				instruction.type = IT_T_MEM_HALF;
				instruction.data.mh.op = (code >> 11) & 1;
				instruction.data.mh.nn = (code >> 6) & 0x1F;
				instruction.data.mh.rb = (code >> 3) & 0x7;
				instruction.data.mh.rd = code & 0x7;	
			}
			return true;

		case 5:
			if(code & (1 << 12))
			{
				if((code & 0x0F00) == 0) // ADD SP, #nn
				{
					instruction.type = IT_T_ADDSP;
					instruction.data.asp.offset = code & 0x7F;
					if(code & (1 << 7)) // Negative
						instruction.data.asp.offset *= -1;	
					return true;
				}
				else if(((code >> 9) & 0x3) == 2) // PUSH/POP
				{
					instruction.type = IT_T_PUSHPOP;
					instruction.data.pp.op = (code >> 11) & 1;
					instruction.data.pp.pclr = (code >> 8) & 1;
					instruction.data.pp.rlist = code & 0xFF;
					return true;
				}
			}
			else // ADD PC/SP
			{
				instruction.type = IT_T_RELATIVE;
				instruction.data.r.sp = (code >> 11) & 1;
				instruction.data.r.rd = (code >> 8) & 0x7;
				instruction.data.r.nn = (code & 0xFF) << 2;
				return true;
			}
			break;

		case 6:
			if((code >> 8) == 0xDF) // SVC
			{
				instruction.type = IT_T_SVC;
				instruction.data.svc.op = code & 0xFF;
			}
			else if((code >> 12) & 1) // B{cond}
			{
				instruction.type = IT_T_BRANCH_COND;
				instruction.data.bc.cond = (code >> 8) & 0xF;
				instruction.data.bc.offset = Unsigned2Signed(code & 0xF, 8) * 2 + 4;
			}
			else // STM/LDM
			{
				instruction.type = IT_T_MULTIPLE;
				instruction.data.m.op = (code >> 11) & 1;
				instruction.data.m.rb = (code >> 8) & 0x7;
				instruction.data.m.rlist = code & 0xFF;
			}
			return true;

		case 7:
		{
			unsigned int op = (code >> 11) & 0x3;
			if(op == 0) // B
			{
				instruction.type = IT_T_BRANCH;
				instruction.data.b.offset = 4 + 2 * Unsigned2Signed(code & 0x7FF, 10);
			}
			else // BL, BLX
			{
				instruction.type = IT_T_BRANCH_LINK;
				if(op == 2) // LR part
				{
					instruction.data.bl.lr = 1;
				}
				else // PC part
				{
					instruction.data.bl.lr = 0;
					instruction.data.bl.x = ((op & 2) == 0);
				}
				instruction.data.bl.nn = code & ((1 << 11) - 1);
			}
			return true;
		}

		default:
			break;
	}

	return false;
}

void PrintInstructionThumb(u_int32_t pcv, const InstructionThumb &instruction)
{
	switch(instruction.type)
	{
		case IT_T_SHIFTED:
			switch(instruction.data.shi.op)
			{
				case 1:
					std::cout << "lsr";
					break;

				case 2:
					std::cout << "asr";
					break;

				default:
					std::cout << "lsl";
					break;
			}
			std::cout << "s ";
			PrintRegister(instruction.data.shi.rd);
			std::cout << ", ";
			PrintRegister(instruction.data.shi.rs);
			std::cout << ", #" << (int)instruction.data.shi.offset;
			return;
		
		case IT_T_ADDSUB:
			if(instruction.data.as.op & 1)
				std::cout << "sub ";
			else
				std::cout << "adds ";
			PrintRegister(instruction.data.as.rd);
			std::cout << ", ";
			PrintRegister(instruction.data.as.rs);
			std::cout << ", ";
			if(instruction.data.as.op & 2) // Immediate
				std::cout << '#' << (int)instruction.data.as.nn;
			else // Register
				PrintRegister(instruction.data.as.nn);
			return;

		case IT_T_IMM:
			switch(instruction.data.im.op)
			{
				case 1:
					std::cout << "cmp ";
					break;
				case 2:
					std::cout << "adds ";
					break;
				case 3:
					std::cout << "subs ";
					break;
				default:
					std::cout << "movs ";
					break;
			}
			PrintRegister(instruction.data.im.rd);
			std::cout << ", #" << (int)instruction.data.im.nn;
			return;

		case IT_T_MEM_IMM:
		{
			u_int32_t offset;
			if(instruction.data.mi.op & 1)
				std::cout << "ldr";
			else
				std::cout << "str";
			if(instruction.data.mi.op & 2) // Byte mode
			{
				std::cout << 'b';
				offset = instruction.data.mi.nn;
			}
			else
				offset = instruction.data.mi.nn << 2;
			std::cout << ' ';
			PrintRegister(instruction.data.mi.rd);
			std::cout << ", [";
			PrintRegister(instruction.data.mi.rb);
			std::cout << ", #" << offset << ']';
			return;
		}

		case IT_T_LDR_PC:
		{
			unsigned int offset = 4 * static_cast<unsigned int>(instruction.data.lp.nn);
			std::cout << "ldr ";
			PrintRegister(instruction.data.lp.rd);
			std::cout << ", [pc, #" << offset << ']';
			return;
		}

		case IT_T_ALU:
		{
			const char *ops[16] = {
				"and",
				"eor",
				"lsl",
				"lsr",
				"asr",
				"adc",
				"sbc",
				"ror",
				"tst",
				"neg",
				"cmp",
				"cmn",
				"orr",
				"mul",
				"bic",
				"mvn"
			};
			std::cout << ops[instruction.data.al.op] << "s ";
			PrintRegister(instruction.data.al.rd);
			std::cout << ", ";
			PrintRegister(instruction.data.al.rs);
			return;
		}

		case IT_T_HIREG:
			switch(instruction.data.hr.op)
			{
				case 1:
					std::cout << "cmp";
					break;
				case 2:
					std::cout << "movs";
					break;
				case 3:
					if(instruction.data.hr.rd & (1 << 3))
						std::cout << "blx";
					else
						std::cout << "bx";
					break;
				default:
					std::cout << "adds";
					break;
			}
			std::cout << ' ';
			if(instruction.data.hr.op != 3)
			{
				PrintRegister(instruction.data.hr.rd);
				std::cout << ", ";
			}
			PrintRegister(instruction.data.hr.rs);
			return;

		case IT_T_MEM_HALF:
			if(instruction.data.mh.op)
				std::cout << "ldr";
			else
				std::cout << "str";
			std::cout << "h ";
			PrintRegister(instruction.data.mh.rd);
			std::cout << ", [";
			PrintRegister(instruction.data.mh.rb);
			std::cout << ", #" << (2 * (int)instruction.data.mh.nn) << ']';
			return;

		case IT_T_MEM_SP:
			if(instruction.data.ms.op)
				std::cout << "ldr ";
			else
				std::cout << "str ";
			PrintRegister(instruction.data.ms.rd);
			std::cout << ", [sp, #" << (4 * (int)instruction.data.ms.nn) << ']';
			return;

		case IT_T_RELATIVE:
			std::cout << "add ";
			PrintRegister(instruction.data.r.rd);
			std::cout << ", ";
			std::cout << (instruction.data.r.sp ? "sp" : "pc") << ", #";
			std::cout << (int)instruction.data.r.nn;
			return;

		case IT_T_ADDSP:
			std::cout << "add sp, #";
			std::cout << (4 * (int)instruction.data.asp.offset);
			return;

		case IT_T_PUSHPOP:
		{
			bool first = true;
			if(instruction.data.pp.op)
				std::cout << "pop ";
			else
				std::cout << "push ";
			std::cout << '{';
			for(unsigned int i = 0; i < 8; ++i)
			{
				if((instruction.data.pp.rlist >> i) & 1)
				{
					if(!first)
						std::cout << ", ";
					PrintRegister(i);
					first = false;
				}
			}
			if(instruction.data.pp.pclr)
			{
				if(!first)
					std::cout << ", ";
				if(instruction.data.pp.op)
					std::cout << "pc";
				else
					std::cout << "lr";
			}
			std::cout << '}';
			return;
		}

		case IT_T_MULTIPLE:
		{
			bool first = true;
			if(instruction.data.m.op)
				std::cout << "ldm";
			else
				std::cout << "stm";
			std::cout << "ia ";
			PrintRegister(instruction.data.m.rb);
			std::cout << "!, {";
			for(unsigned int i = 0; i < 8; ++i)
			{
				if((instruction.data.pp.rlist >> i) & 1)
				{
					if(!first)
						std::cout << ", ";
					PrintRegister(i);
					first = false;
				}
			}
			std::cout << '}';
			return;
		}

		case IT_T_BRANCH_COND:
			std::cout << 'b';
			PrintCondition(instruction.data.bc.cond);
			std::cout << ' ';
			PrintHex(pcv + instruction.data.bc.offset);
			return;

		case IT_T_SVC:
			std::cout << "svc " << (int)instruction.data.svc.op;
			return;

		case IT_T_BRANCH:
			std::cout << "b ";
			PrintHex(pcv + instruction.data.b.offset);
			return;

		case IT_T_BRANCH_LINK:
			if(!instruction.data.bl.lr)
			{
				unsigned int offset = instruction.data.bl.nn;
				u_int32_t tmp = (blAddress | (offset << 1)) + 2;
				int32_t jmp = Unsigned2Signed(tmp, 22);
				std::cout << "bl";
				if(instruction.data.bl.x)
					std::cout << 'x';
				std::cout << ' ';
				PrintHex(pcv + jmp);
			}
			else
				blAddress = (instruction.data.bl.nn << 12) + 4;
			return;

		default:
			break;
	}
	
	std::cout << "I was too lazy to implement disassembly for this instruction" ;
}

