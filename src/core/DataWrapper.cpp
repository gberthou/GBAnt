#include "DataWrapper.h"

DataWrapper::DataWrapper(u_int32_t *p):
	ptr(p),
	written(false)
{
}

DataWrapper::~DataWrapper()
{
}

void DataWrapper::Set(u_int32_t value)
{
	*ptr = value;
	written = true;
}

u_int32_t DataWrapper::Get(void) const
{
	return *ptr;
}

bool DataWrapper::IsWritten(void) const
{
	return written;
}

