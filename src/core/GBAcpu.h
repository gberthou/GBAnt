#ifndef GBA_CPU_H
#define GBA_CPU_H

#include "ARMcpu.h"
#include "irq.h"
#include "../peripherals/peripherals.h"

class GBAcpu : public ARMcpu
{
	public:
		GBAcpu();
		virtual ~GBAcpu();

		bool LoadGBA(const char *filename);		
		virtual void Run(void);

		void TriggerInterrupt(GBA_InterruptSource source);

	protected:
		virtual void runStep(void);
		virtual void onClock(void);
		virtual bool interruptsEnabled(void);

		PhysicalMemory *cartridge;
		
		// Peripherals
		Lcd *lcd;

	friend class DMA;
};

#endif

