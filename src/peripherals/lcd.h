#ifndef LCD_H
#define LCD_H

#include <sys/types.h>

#include "../core/Memory.h"
#include "video.h"

#define LCD_BG_COUNT 4
#define LCD_ROTSCALE_BG_COUNT 2

struct LcdBgOffset
{
	u_int16_t x;
	u_int16_t y;
};

struct LcdBgRotScale
{
	u_int16_t a;
	u_int16_t b;
	u_int16_t c;
	u_int16_t d;
	u_int32_t refX;
	u_int32_t refY;
};

struct LcdData
{
	u_int16_t dispcnt;
	u_int16_t downtknow;
	u_int16_t dispstat;
	u_int16_t vcount;
	u_int16_t bgcnt[LCD_BG_COUNT];
	LcdBgOffset offset[LCD_BG_COUNT];
	LcdBgRotScale rotscale[LCD_ROTSCALE_BG_COUNT];
	u_int16_t win0w;
	u_int16_t win1w;
	u_int16_t win0h;
	u_int16_t win1h;
	u_int16_t winin;
	u_int16_t winout;
	u_int16_t mosaic;
	u_int16_t pad;
	u_int16_t bldcnt;
	u_int16_t bldalpha;
	u_int16_t bldy;
	u_int16_t pad1;
};

class Lcd : public Memory
{
	public:	
		Lcd(u_int32_t bAddress, u_int32_t eAddress);
		virtual ~Lcd();

		virtual bool Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess = MA32);
		virtual bool Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess = MA32);

		void Trigger(void);
		void OnClock(void);
		void Render(void);

	private:
		void printStatus(void);
		
		LcdData lcd;
		Background backgrounds[LCD_BG_COUNT];
		u_int32_t cmpt;
};

#endif

