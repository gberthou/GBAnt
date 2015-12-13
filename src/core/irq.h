#ifndef IRQ_H
#define IRQ_H

#include <sys/types.h>

enum GBA_InterruptSource
{
	GBA_IS_LCD_VBLANK   = 1,
	GBA_IS_LCD_HBLANK   = 2,
	GBA_IS_LCD_VCOUNTER = 4,
	GBA_IS_TIMER0       = 8,
	GBA_IS_TIMER1       = 16,
	GBA_IS_TIMER2       = 32,
	GBA_IS_TIMER3       = 64,
	GBA_IS_SERIAL       = 128,
	GBA_IS_DMA0         = 256,
	GBA_IS_DMA1         = 512,
	GBA_IS_DMA2         = 1024,
	GBA_IS_DMA3         = 2048,
	GBA_IS_KEYPAD       = 4096,
	GBA_IS_GAMEPAK      = 8192
};

#endif

