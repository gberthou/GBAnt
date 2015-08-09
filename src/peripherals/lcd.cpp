#include <iostream>
#include <cstdlib>
#include <cstring>

#include "lcd.h"

#define HBLANK_STEP_CYCLE_COUNT 4
//#define VBLANK_STEP_CYCLE_COUNT (HBLANK_STEP_CYCLE_COUNT * 308)
//#define VBLANK_STEP_CYCLE_COUNT 100
#define VBLANK_STEP_CYCLE_COUNT 4

Lcd::Lcd(u_int32_t bAddress, u_int32_t eAddress):
	Memory(bAddress, eAddress),
	cmpt(0)
{
	memset(&lcd, 0, sizeof(lcd));
}

Lcd::~Lcd()
{
}

bool Lcd::Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess)
{
	if(address >= bAddress && address <= eAddress)
	{
		if(memAccess == MA16)
		{
			u_int16_t *data = reinterpret_cast<u_int16_t*>(&lcd);
			data[(address - bAddress) / 2] = value;
		}
		else // MA32
		{
			u_int32_t *data = reinterpret_cast<u_int32_t*>(&lcd);
			data[(address - bAddress) / 4] = value;
		}

		Trigger();
		printStatus();

		return true;
	}
	return false;
}

bool Lcd::Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess)
{
	if(address >= bAddress && address <= eAddress)
	{
		if(memAccess == MA8)
		{
			u_int8_t *data = reinterpret_cast<u_int8_t*>(&lcd);
			value = data[address - bAddress];
		}
		else if(memAccess == MA16)
		{
			u_int16_t *data = reinterpret_cast<u_int16_t*>(&lcd);
			value = data[(address - bAddress) / 2];
		}
		else // MA32
		{
			u_int32_t *data = reinterpret_cast<u_int32_t*>(&lcd);
			value = data[(address - bAddress) / 4];
		}
		return true;
	}
	return false;
}

void Lcd::Trigger(void)
{
	if((lcd.dispstat >> 3) & 7)
		std::cout << "LCD IRQ REQUIRED" << std::endl;
}

void Lcd::OnClock(void)
{
	++cmpt;
	lcd.vcount = (lcd.vcount + cmpt / VBLANK_STEP_CYCLE_COUNT) & 0xFF;
	cmpt %= VBLANK_STEP_CYCLE_COUNT;

	std::cout << "VCOUNT = " << lcd.vcount << std::endl;
}

void Lcd::Render(void)
{
}

void Lcd::printStatus(void)
{
	std::cout << "DISPCNT: " << lcd.dispcnt << std::endl;
	for(unsigned int i = 0; i < LCD_BG_COUNT; ++i)
	{
		if(lcd.dispcnt & (1 << (8 + i)))
		{
			std::cout << "BG" << i << " enabled!" << std::endl;
		}
	}
}

