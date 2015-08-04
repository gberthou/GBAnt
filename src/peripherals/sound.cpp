#include <iostream>

#include "sound.h"

Sound::Sound(u_int32_t bAddress, u_int32_t eAddress):
	Memory(bAddress, eAddress)
{
}

Sound::~Sound()
{
}

bool Sound::Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess)
{
	if(address >= bAddress && address <= eAddress)
	{
		if(memAccess == MA16)
		{
			u_int16_t *data = reinterpret_cast<u_int16_t*>(&snd);
			data[(address - bAddress) / 2] = value;
		}
		else // MA32
		{
			u_int32_t *data = reinterpret_cast<u_int32_t*>(&snd);
			data[(address - bAddress) / 4] = value;
		}

		Trigger();

		return true;
	}
	return false;
}

bool Sound::Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess)
{
	if(address >= bAddress && address <= eAddress)
	{
		if(memAccess == MA16)
		{
			u_int16_t *data = reinterpret_cast<u_int16_t*>(&snd);
			value = data[(address - bAddress) / 2];
		}
		else // MA32
		{
			u_int32_t *data = reinterpret_cast<u_int32_t*>(&snd);
			value = data[(address - bAddress) / 4];
		}

		return true;
	}
	return false;
}
	
void Sound::Trigger(void)
{
	if((snd.s1.x >> 15) & 1)
	{
		std::cout << "SND channel 1 started" << std::endl;
	}
	if((snd.s2.h >> 15) & 1)
	{
		std::cout << "SND channel 2 started" << std::endl;
	}
	if((snd.s3.x >> 15) & 1)
	{
		std::cout << "SND channel 3 started" << std::endl;
	}
	if((snd.s4.h >> 15) & 1)
	{
		std::cout << "SND channel 4 started" << std::endl;
	}
}

