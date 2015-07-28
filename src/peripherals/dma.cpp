#include <iostream>
#include <cstdlib>
#include <cstring>

#include "dma.h"

DMA::DMA(u_int32_t bAddress, u_int32_t eAddress, GBAcpu *gb):
	Memory(bAddress, eAddress),
	gba(gb)
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
		for(unsigned int i = 0; i < (memAccess == MA32 ? 2 : 1); ++i)
		{
			unsigned int shift = i * 16;
			std::cerr << "DMA write @";
			PrintHex(address + i * 2, 32, std::cerr);
			std::cerr << ", ";
			PrintHex((value >> shift) & 0xFFFF, memAccess == MA32 ? 32 : 16, std::cerr);
			std::cerr << std::endl;
		}

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
		if(((channels[i].control >> 15) & 1) != 0 // Channel is enabled
		&& (((channels[i].control >> 12) & 0x3) == source)) // Channel is triggered 
		{
			memcpy(tmp + i, channels + i, sizeof(DMA_Channel));

			std::cout << "Requested DMA transfer on channel " << i << std::endl;
			std::cout << " SRC: ";
			PrintHex(channels[i].src);
			std::cout << std::endl << " DST: ";
			PrintHex(channels[i].dst);
			std::cout << std::endl << " SIZ: ";
			PrintHex(channels[i].wordCount);
			std::cout << std::endl;
		
			while(step(i));
		}
	}
}

bool DMA::step(unsigned int index)
{
	u_int32_t value;
	u_int16_t ctrl = tmp[index].control;
	unsigned int bitcount = ((ctrl >> 10) & 1) ? 32 : 16;
	unsigned int srcCtrl = (ctrl >> 7) & 3;
	unsigned int dstCtrl = (ctrl >> 5) & 3;

	gba->mem.Read(tmp[index].src, value, bitcount == 16 ? MA16 : MA32);
	gba->mem.Write(tmp[index].dst, value, bitcount == 16 ? MA16 : MA32);

	if(srcCtrl == 0) // Increment
		tmp[index].src += bitcount / 8;	
	else if(srcCtrl == 1) // Decrement
		tmp[index].src -= bitcount / 8;	
	else if(srcCtrl == 3) // Prohibited
		std::cerr << "DMA src control prohibited" << std::endl;

	if(dstCtrl == 0 || dstCtrl == 3) // Increment
	{
		tmp[index].dst += bitcount / 8;
		if(dstCtrl == 3)
			std::cerr << "DMA dst reload not supported yet" << std::endl;
	}
	else if(dstCtrl == 1) // Decrement
		tmp[index].dst -= bitcount / 8;	

	return --tmp[index].wordCount > 0;
}

