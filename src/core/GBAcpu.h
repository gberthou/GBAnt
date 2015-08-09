#ifndef GBA_CPU_H
#define GBA_CPU_H

#include "ARMcpu.h"
#include "../peripherals/peripherals.h"

class GBAcpu : public ARMcpu
{
	public:
		GBAcpu();
		virtual ~GBAcpu();

		bool LoadGBA(const char *filename);		
		virtual void Run(void);

	protected:
		virtual void onClock(void);

		PhysicalMemory *cartridge;
		
		// Peripherals
		Lcd *lcd;

	friend class DMA;
};

#endif

