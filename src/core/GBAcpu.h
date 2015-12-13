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

		virtual void RunNextInstruction(void);
		
		bool LoadBios(const char *filename);		
		bool LoadGBA(const char *filename);		
		
		void TriggerInterrupt(GBA_InterruptSource source);

		// TESTS
		bool RunTestStack(void);

	protected:
		virtual void onClock(void);
		virtual bool interruptsEnabled(void);
		
		bool loadFileToMemory(const char *filename, PhysicalMemory *memory, u_int32_t baseAddress);		
		
		// BIOS
		virtual void biosCall(u_int8_t op);
		void biosIntrWait(void);
		void biosVBlankIntrWait(void);
		void biosCpuSet(void);


		PhysicalMemory *bios;
		PhysicalMemory *cartridge;
		
		// Peripherals
		Lcd *lcd;

	friend class DMA;
};

#endif

