#ifndef TIMER_H
#define TIMER_H

#include "../core/Memory.h"

#define TIMER_CHANNELS_COUNT 4 

struct TimerChannel
{
	u_int16_t counter;
	u_int16_t ctrl;
};

class Timer : public Memory
{
	public:
		Timer(u_int32_t bAddress, u_int32_t eAddress);
		virtual ~Timer();

		virtual bool Write(u_int32_t address, u_int32_t value, MemoryAccess memAccess = MA32);
		virtual bool Read(u_int32_t address, u_int32_t &value, MemoryAccess memAccess = MA32);

		void Trigger(void);
	
	private:
		TimerChannel channels[TIMER_CHANNELS_COUNT];
};

#endif

