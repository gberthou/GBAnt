#ifndef SOUND_H
#define SOUND_H

#include <sys/types.h>

#include "../core/Memory.h"

#define SND_WAVE_RAM_SIZE 0x10

struct Sound1Cnt
{
	u_int16_t l;
	u_int16_t h;
	u_int16_t x;
	u_int16_t pad;
};

typedef Sound1Cnt Sound3Cnt;
typedef Sound1Cnt SoundCnt;

struct Sound2Cnt
{
	u_int16_t l;
	u_int16_t pad;
	u_int16_t h;
	u_int16_t pad1;
};

struct Sound4Cnt
{
	u_int16_t l;
	u_int16_t pad;
	u_int16_t h;
	u_int16_t pad1;
};

struct SoundBias
{
	u_int16_t pwmctrl;
	u_int16_t pad[3];
};

struct SoundData
{
	Sound1Cnt s1;
	Sound2Cnt s2;
	Sound3Cnt s3;
	Sound4Cnt s4;
	SoundCnt cnt;
	SoundBias bias;
	u_int8_t wav[SND_WAVE_RAM_SIZE];
	u_int32_t fifoA;
	u_int32_t fifoB;
	u_int32_t pad;
};

class Sound : public Memory
{
	public:
		Sound(u_int32_t bAddress, u_int32_t eAddress);
		virtual ~Sound();

		virtual bool Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess = MA32);
		virtual bool Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess = MA32);

		void Trigger(void);

	private:
		SoundData snd;
};

#endif

