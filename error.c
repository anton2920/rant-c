#include "u.h"
#include "builtin.h"

#include "arena.h"
#include "assert.h"
#include "atomic.h"
#include "error.h"
#include "slice.h"

static E ErrorsArena[256];
static int	ErrorsArenaLast = -1;

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
	if (err != nil) {
		/* NOTE(anton2920): allocation failed, but we still need to return error. */
		int	i = AtomicAddInt32(&ErrorsArenaLast, 1);
		assert(((uint64) i) < (sizeof(ErrorsArena) / sizeof(ErrorsArena[0])));
		err = &ErrorsArena[i];
	}

	err->Message = UnsafeCString(msg);
	err->Code = (int)code;
	err->Error = EError;
	return (error)err;
}


