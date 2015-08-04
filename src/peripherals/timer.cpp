#include <cstring>
#include <iostream>
#include <cstdlib>

#include "timer.h"

Timer::Timer(u_int32_t bAddress, u_int32_t eAddress):
	Memory(bAddress, eAddress)
{
	memset(channels, 0, sizeof(channels));
}

Timer::~Timer()
{
}

bool Timer::Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess)
{
	if(address >= bAddress && address <= eAddress)
	{
		if(memAccess == MA16)
		{
			u_int16_t *data = reinterpret_cast<u_int16_t*>(channels);
			data[(address - bAddress) / 2] = value;
		}
		else // MA32
		{
			u_int32_t *data = reinterpret_cast<u_int32_t*>(channels);
			data[(address - bAddress) / 4] = value;
		}

		Trigger();

		return true;
	}
	return false;
}

bool Timer::Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess)
{
	if(address >= bAddress && address <= eAddress)
	{
		if(memAccess == MA16)
		{
			u_int16_t *data = reinterpret_cast<u_int16_t*>(channels);
			value = data[(address - bAddress) / 2];
		}
		else // MA32
		{
			u_int32_t *data = reinterpret_cast<u_int32_t*>(channels);
			value = data[(address - bAddress) / 4];
		}
		return true;
	}
	return false;
}

void Timer::Trigger(void)
{
	for(unsigned int i = 0; i < TIMER_CHANNELS_COUNT; ++i)
	{
		if((channels[i].ctrl >> 7) & 1)
		{
			std::cout << "TIMER " << i << " enabled" << std::endl;
			if((channels[i].ctrl >> 6) & 1)
				std::cout << "TIMER " << i << " IRQ enabled" << std::endl;
		}
	}
}

