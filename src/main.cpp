#include <iostream>
#include <cstdlib>

#include "core/GBAcpu.h"

//#define TEST_MODE

#ifndef TEST_MODE
int main(int argc, char **argv)
{
	(void) argv;
	if(argc > 1)
	{
		GBAcpu cpu;
		if(cpu.LoadGBA(argv[1]))
		{
			cpu.Run();
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

