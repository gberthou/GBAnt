#ifndef INSTRUCTION_THUMB_H
#define INSTRUCTION_THUMB_H

#include <sys/types.h>

typedef struct
{
	u_int8_t op;
	u_int8_t offset;
	u_int8_t rs;
	u_int8_t rd;
} InstTShifted;

typedef struct
{
	u_int8_t op;
	u_int8_t nn;
	u_int8_t rs;
	u_int8_t rd;
} InstTAddSub;

typedef struct
{
	u_int8_t op;
	u_int8_t rd;
	u_int8_t nn;
} InstTImm;

typedef struct
{
	u_int8_t op;
	u_int8_t rs;
	u_int8_t rd;
} InstTAlu;

typedef struct
{
	u_int8_t op;
	u_int8_t rs;
	u_int8_t rd;
} InstTHiReg;

typedef struct
{
	u_int8_t rd;
	u_int8_t nn;
} InstTLdrPc;

typedef struct
{
	u_int8_t op;
	u_int8_t nn;
	u_int8_t rb;
	u_int8_t rd;
} InstTMemImm;

typedef struct
{
	u_int8_t op;
	u_int8_t ro;
	u_int8_t rb;
	u_int8_t rd;
} InstTMemSpecial;

typedef struct
{
	u_int8_t op;
	u_int8_t nn;
	u_int8_t rb;
	u_int8_t rd;
} InstTMemHalf;

typedef struct
{
	u_int8_t op;
	u_int8_t rd;
	u_int8_t nn;
} InstTMemSP;

typedef struct
{
	u_int8_t sp;
	u_int8_t rd;
	u_int8_t nn;
} InstTRelative;

typedef struct
{
	int8_t offset;
} InstTAddSP;

typedef struct
{
	u_int8_t op;
	u_int8_t pclr;
	u_int8_t rlist;
} InstTPushPop;

typedef struct
{
	u_int8_t op;
	u_int8_t rb;
	u_int8_t rlist;
} InstTMultiple;

typedef struct
{
	u_int8_t cond;
	int16_t offset;
} InstTBranchCond;

typedef struct
{
	u_int8_t op;
} InstTSVC;

typedef struct
{
	int16_t offset;
} InstTBranch;

typedef struct
{
	u_int8_t lr;
	u_int8_t x;
	u_int16_t nn;
} InstTBLink;

typedef union
{
	InstTShifted    shi;
	InstTAddSub     as;
	InstTImm        im;
	InstTAlu        al;
	InstTHiReg      hr;
	InstTLdrPc      lp;
	InstTMemImm     mi;
	InstTMemSpecial s;
	InstTMemHalf    mh;
	InstTMemSP      ms;
	InstTRelative   r;
	InstTAddSP      asp;
	InstTPushPop    pp;
	InstTMultiple   m;
	InstTBranchCond bc;
	InstTSVC        svc;
	InstTBranch     b;
	InstTBLink      bl;
} InstructionTData;

typedef enum
{
	IT_T_SHIFTED,
	IT_T_ADDSUB,
	IT_T_IMM,
	IT_T_ALU,
	IT_T_HIREG,
	IT_T_LDR_PC,
	IT_T_MEM_IMM,
	IT_T_MEM_SPECIAL,
	IT_T_MEM_HALF,
	IT_T_MEM_SP,
	IT_T_RELATIVE,
	IT_T_ADDSP,
	IT_T_PUSHPOP,
	IT_T_MULTIPLE,
	IT_T_BRANCH_COND,
	IT_T_SVC,
	IT_T_BRANCH,
	IT_T_BRANCH_LINK,
	IT_T_UNKNOWN
} InstructionTType;

typedef struct
{
	InstructionTType type;
	InstructionTData data;
} InstructionThumb;

bool DecodeInstructionThumb(u_int16_t code, InstructionThumb &instruction);
void PrintInstructionThumb(u_int32_t pcv, const InstructionThumb &instruction);

#endif

