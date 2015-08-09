#include "video.h"

const unsigned int BACKGROUND_MAX_SIZE = 1024;

Background::Background()
{
	pixels = new u_int32_t[BACKGROUND_MAX_SIZE];
}

Background::~Background()
{
	delete [] pixels;
}

