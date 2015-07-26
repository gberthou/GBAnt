#include <iostream>
#include <cstdlib>
#include <cstring>

#include "dma.h"

DMA::DMA(u_int32_t bAddress, u_int32_t eAddress):
	Memory(bAddress, eAddress)
{
	memset(channels, 0, sizeof(channels));
}

DMA::~DMA()
{
}

bool DMA::Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess)
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

		Trigger(DMA_TS_IMMEDIATLY);
		return true;
	}
	return false;
}

bool DMA::Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess)
{
	if(address >= bAddress && address <= eAddress)
	{
		if(memAccess == MA16)
		{
			u_int16_t *data = reinterpret_cast<u_int16_t*>(channels);
			value = *(data + (address - bAddress) / 2);
		}
		else // MA32
		{
			u_int32_t *data = reinterpret_cast<u_int32_t*>(channels);
			value = *(data + (address - bAddress) / 4);
		}

		return true;
	}
	return false;
}

void DMA::Trigger(DMA_TriggerSource source)
{
	unsigned int i = DMA_CHANNELS_COUNT;
	while(i)
	{
		--i;
		if((channels[i].control & 1) != 0 // Channel is enabled
		&& (((channels[i].control >> 12) & 0x3) == source)) // Channel is triggered 
		{
			memcpy(tmp + i, channels + i, sizeof(DMA_Channel));

			// TODO
			std::cout << "Requested DMA transfer on channel " << i << std::endl;
			std::cout << " SRC: ";
			PrintHex(channels[i].src);
			std::cout << std::endl << " DST: ";
			PrintHex(channels[i].dst);
			std::cout << std::endl << " SIZ: ";
			PrintHex(channels[i].wordCount);
			std::cout << std::endl;
		}
	}
}

