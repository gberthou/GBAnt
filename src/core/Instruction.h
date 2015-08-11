#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <sys/types.h>

typedef struct
{
	u_int8_t cond;
	u_int8_t op;
	u_int8_t s;
	u_int8_t rn;
	u_int8_t rd;
	u_int8_t shift;
	u_int8_t typ;
	u_int8_t rm;
} InstDataProc1;

typedef struct
{
	u_int8_t cond;
	u_int8_t op;
	u_int8_t s;
	u_int8_t rn;
	u_int8_t rd;
	u_int8_t rs;
	u_int8_t typ;
	u_int8_t rm;
} InstDataProc2;

typedef struct
{
	u_int8_t cond;
	u_int8_t op;
	u_int8_t s;
	u_int8_t rn;
	u_int8_t rd;
	u_int8_t shift;
	u_int8_t immediate;
} InstDataProc3;

typedef struct
{
	u_int8_t cond;
	u_int8_t p;
	u_int8_t field;
	u_int8_t rd;
	u_int8_t shift;
	u_int8_t immediate;
} InstPSRImm;

typedef struct
{
	u_int8_t cond;
	u_int8_t p;
	u_int8_t l;
	u_int8_t field;
	u_int8_t rd;
	u_int8_t rm;
} InstPSRReg;

typedef struct
{
	u_int8_t cond;
	u_int8_t l;
	u_int8_t rn;
} InstBranchEx;

typedef struct
{
	u_int16_t immediate;
	u_int8_t immed;
} InstBKPT9;

typedef struct
{
	u_int8_t cond;
	u_int8_t rd;
	u_int8_t rm;
} InstCLZ9;

typedef struct
{
	u_int8_t cond;
	u_int8_t op;
	u_int8_t rn;
	u_int8_t rd;
	u_int8_t rm;
} InstQALU9;

typedef struct
{
	u_int8_t cond;
	u_int8_t a;
	u_int8_t s;
	u_int8_t rd;
	u_int8_t rn;
	u_int8_t rs;
	u_int8_t rm;
} InstMultiply;

typedef struct
{
	u_int8_t cond;
	u_int8_t u;
	u_int8_t a;
	u_int8_t s;
	u_int8_t rdhi;
	u_int8_t rdlo;
	u_int8_t rs;
	u_int8_t rm;
} InstMulLong;

typedef struct
{
	u_int8_t cond;
	u_int8_t op;
	u_int8_t rdhi;
	u_int8_t rdlo;
	u_int8_t rs;
	u_int8_t y;
	u_int8_t x;
	u_int8_t rm;
} InstMulHalf9;

typedef struct
{
	u_int8_t cond;
	u_int8_t b;
	u_int8_t rn;
	u_int8_t rd;
	u_int8_t rm;
} InstTransSwp12;

typedef struct
{
	u_int8_t cond;
	u_int8_t p;
	u_int8_t u;
	u_int8_t w;
	u_int8_t l;
	u_int8_t rn;
	u_int8_t rd;
	u_int8_t s;
	u_int8_t h;
	u_int8_t rm;
} InstTransReg10;

typedef struct
{
	u_int8_t cond;
	u_int8_t p;
	u_int8_t u;
	u_int8_t w;
	u_int8_t l;
	u_int8_t rn;
	u_int8_t rd;
	u_int8_t offsetH;
	u_int8_t s;
	u_int8_t h;
	u_int8_t offsetL;
} InstTransImm10;

typedef struct
{
	u_int8_t cond;
	u_int8_t p;
	u_int8_t u;
	u_int8_t b;
	u_int8_t w;
	u_int8_t l;
	u_int8_t rn;
	u_int8_t rd;
	u_int16_t offset;
} InstTransImm9;

typedef struct
{
	u_int8_t cond;
	u_int8_t p;
	u_int8_t u;
	u_int8_t b;
	u_int8_t w;
	u_int8_t l;
	u_int8_t rn;
	u_int8_t rd;
	u_int8_t shift;
	u_int8_t typ;
	u_int8_t rm;
} InstTransReg9;

typedef struct
{
	u_int8_t cond;
	u_int8_t p;
	u_int8_t u;
	u_int8_t s;
	u_int8_t w;
	u_int8_t l;
	u_int8_t rn;
	u_int16_t rlist;
} InstBlockTrans;

typedef struct
{
	u_int8_t cond;
	u_int8_t l;
	int32_t offset;
} InstBranch;

typedef struct
{
	u_int8_t cond;
} InstSWI;

// TODO: Coprocessor instructions

typedef union
{
	InstDataProc1  dp1;
	InstDataProc2  dp2;
	InstDataProc3  dp3;
	InstPSRImm     psri;
	InstPSRReg     psrr;
	InstBranchEx   bx;
	InstBKPT9      b9;
	InstCLZ9       c9;
	InstQALU9      q9;
	InstMultiply   m;
	InstMulLong    ml;
	InstMulHalf9   mh;
	InstTransSwp12 ts12;
	InstTransReg10 tr10;
	InstTransImm10 ti10;
	InstTransImm9  ti9;
	InstTransReg9  tr9;
	InstBlockTrans bt;
	InstBranch     b;
	InstSWI        swi;
} InstructionData;

typedef enum
{
	IT_DATA_PROC1,
	IT_DATA_PROC2,
	IT_DATA_PROC3,
	IT_PSR_IMM,
	IT_PSR_REG,
	IT_BRANCH_EX,
	IT_BKPT9,
	IT_CLZ9,
	IT_QUALU9,
	IT_MULTIPLY,
	IT_MUL_LONG,
	IT_MUL_HALF9,
	IT_TRANS_SWP12,
	IT_TRANS_REG10,
	IT_TRANS_IMM10,
	IT_TRANS_IMM9,
	IT_BLOCK_TRANS,
	IT_BRANCH,
	IT_SWI,
	IT_UNKNOWN
} InstructionType;

typedef struct
{
	InstructionType type;
	InstructionData data;
} Instruction;

bool DecodeInstruction(u_int32_t code, Instruction &instruction);
void PrintInstruction(const Instruction &instruction);

#endif

