#ifndef GBA_CPU_H
#define GBA_CPU_H

#include "ARMcpu.h"
#include "../peripherals/peripherals.h"

enum GBA_InterruptSource
{
	GBA_IS_LCD_VBLANK,
	GBA_IS_LCD_HBLANK,
	GBA_IS_LCD_VCOUNTER_MATCH,
	GBA_IS_TIMER0,
	GBA_IS_TIMER1,
	GBA_IS_TIMER2,
	GBA_IS_TIMER3,
	GBA_IS_SERIAL,
	GBA_IS_DMA0,
	GBA_IS_DMA1,
	GBA_IS_DMA2,
	GBA_IS_DMA3,
	GBA_IS_KEYPAD,
	GBA_IS_GAMEPAK
};

class GBAcpu : public ARMcpu
{
	public:
		GBAcpu();
		virtual ~GBAcpu();

		bool LoadGBA(const char *filename);		
		virtual void Run(void);

		void TriggerInterrupt(GBA_InterruptSource source);

	protected:
		virtual void onClock(void);
		bool interruptsEnabled(void);

		PhysicalMemory *cartridge;
		
		// Peripherals
		Lcd *lcd;

	friend class DMA;
};

#endif

