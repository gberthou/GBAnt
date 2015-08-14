#ifndef DATAWRAPPER_H
#define DATAWRAPPER_H

#include <sys/types.h>

class DataWrapper
{
	public:
		DataWrapper(u_int32_t *ptr);
		virtual ~DataWrapper();

		void Set(u_int32_t value);
		u_int32_t Get(void) const;
		bool IsWritten(void) const;

	protected:
		u_int32_t *ptr;
		bool written;
};

#endif

