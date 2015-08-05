#include <iostream>
#include <cstring>

#include "lcd.h"

Lcd::Lcd(u_int32_t bAddress, u_int32_t eAddress):
	Memory(bAddress, eAddress)
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
		return true;
	}
	return false;
}

bool Lcd::Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess)
{
	if(address >= bAddress && address <= eAddress)
	{
		if(memAccess == MA16)
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

}

