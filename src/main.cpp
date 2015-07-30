#include <iostream>
#include <cstdlib>

#include "core/GBAcpu.h"

#include "peripherals/sound.h"

int main(int argc, char **argv)
{
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

