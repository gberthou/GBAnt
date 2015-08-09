#ifndef VIDEO_H
#define VIDEO_H

#include <sys/types.h>

class Background
{
	public:
		Background();
		virtual ~Background();

	private:
		u_int32_t *pixels;
};

#endif

