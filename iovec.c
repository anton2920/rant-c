#include "u.h"
#include "builtin.h"

#include "iovec.h"

#include <sys/uio.h>

uint64 CStringLength(char *);

Iovec
IovecForCString(char *cstr)
{
	Iovec iov;

	iov.iov_base = cstr;
	iov.iov_len = CStringLength(cstr);

	return iov;
}


