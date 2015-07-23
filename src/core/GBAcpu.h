#ifndef GBA_CPU_H
#define GBA_CPU_H

#include "ARMcpu.h"

class GBAcpu : public ARMcpu
{
	public:
		GBAcpu();
		virtual ~GBAcpu();

		bool LoadGBA(const char *filename);		
		virtual void Run(void);

	protected:
		PhysicalMemory *cartridge;
};

#endif

