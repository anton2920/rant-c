#include "u.h"
#include "builtin.h"

#include "iovec.h"

#include <sys/uio.h>

Iovec
IovecForByteSlice(slice s)
{
	Iovec iov;

	iov.iov_base = s.base;
	iov.iov_len = s.len;

	return iov;
}


Iovec
IovecForCString(char *cstr)
{
	return IovecForString(UnsafeCString(cstr));
}


Iovec
IovecForString(string s)
{
	Iovec iov;

	iov.iov_base = s.base;
	iov.iov_len = s.len;

	return iov;
}


