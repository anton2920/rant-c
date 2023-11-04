#include "u.h"
#include "string.h"

#include "assert.h"
#include "error.h"

#include "atomic.h"

Error ErrorsArena[256];
int	LastErrorsArenaIndex = -1;

Error *NewError(char *msg, int code)
{
	int	i = AtomicAddInt32(&LastErrorsArenaIndex, 1);
	Error * err;

	assert(((uint64) i) < (sizeof(ErrorsArena) / sizeof(ErrorsArena[0])));
	err = &ErrorsArena[i];
	err->Message = StringFromCString(msg);
	err->Code = code;
	return err;
}


void
SetOptionalError(Error **err, char *msg, int code)
{
	if (err != nil) {
		*err = NewError(msg, code);
	}
}


