#ifndef CORE_UTILS_H
#define CORE_UTILS_H

#include <iostream>
#include <sys/types.h>

void PrintHex(u_int32_t x, unsigned int bitcount = 32, std::ostream &out = std::cout);
void PrintRegister(u_int8_t reg);
void PrintCondition(u_int8_t condition);
int32_t Unsigned2Signed(u_int32_t x, unsigned int bitcount);

#endif

