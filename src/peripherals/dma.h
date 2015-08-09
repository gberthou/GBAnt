#ifndef DMA_H
#define DMA_H

#include "../core/Memory.h"
#include "../core/utils.h"

#define DMA_CHANNELS_COUNT 4

struct DMA_Channel
{
	u_int32_t src;
	u_int32_t dst;
	u_int16_t wordCount;
	u_int16_t control;
};

enum DMA_TriggerSource
{
	DMA_TS_IMMEDIATLY,
	DMA_TS_VBLANK,
	DMA_TS_HBLANK,
	DMA_TS_SPECIAL
};

class GBAcpu;

class DMA : public Memory
{
	public:
		DMA(u_int32_t bAddress, u_int32_t eAddress, GBAcpu *gba);
		virtual ~DMA();
		
		virtual bool Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess = MA32);
		virtual bool Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess = MA32);

		void Trigger(DMA_TriggerSource source);

	private:
		bool step(unsigned int index);
		
		GBAcpu * const gba;
		DMA_Channel channels[DMA_CHANNELS_COUNT];
		DMA_Channel tmp[DMA_CHANNELS_COUNT];
};

#endif

