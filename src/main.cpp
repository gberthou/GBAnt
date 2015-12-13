#include <iostream>
#include <cstdlib>

#include "core/GBAcpu.h"

//#define TEST_MODE

#ifndef TEST_MODE
int main(int argc, char **argv)
{
	(void) argv;
	if(argc > 2)
	{
		GBAcpu cpu;
		if(cpu.LoadBios(argv[2]) && cpu.LoadGBA(argv[1]))
		{
			for(;;)
				cpu.RunNextInstruction();
		}
	}

	return EXIT_SUCCESS;
}

#else
int main(void)
{
	GBAcpu cpu;
	cpu.RunTestStack();
	return EXIT_SUCCESS;
}

#endif

