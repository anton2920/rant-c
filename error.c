#include "u.h"
#include "builtin.h"
#include "runtime.h"

#include "assert.h"
#include "error.h"
#include "slice.h"

string
EError(error e)
{
	E * err = (E * )e;
	char	buf[512];
	int	n;
	slice s;

	s = UnsafeSlice(buf, sizeof(buf));
	n = copy(s, Slice(err->Message));
	buf[n++] = ' ';

	if (err->Code != 0) {
		n += SlicePutInt(SliceLeft(s, n), err->Code);
	}

	return slicebytetostring(SliceRight(s, n));
}


error
SyscallError(char *msg, uintptr code)
{
	E * err;

	if (code == 0) {
		return nil;
	}

	err = newobject(E);
	assert(err != nil);

	err->Message = UnsafeCString(msg);
	err->Code = (int)code;
	err->Error = EError;
	return (error)err;
}


