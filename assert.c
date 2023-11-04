#include "u.h"
#include "slice.h"
#include "string.h"

#include "syscall.h"

void
_assert_fail(char *expr, char *file, int line)
{
	char	buffer[1024];
	int	ndigits;
	uint64 n;
	Slice s;

	/* file:line: Assertion `expr' failed. */
	s = SliceFrom(buffer, sizeof(buffer));

	n = SliceWriteCString(s, file);
	s = SliceLeft(s, n);

	n = SliceWriteCString(s, ":");
	s = SliceLeft(s, n);

	ndigits = SlicePutInt(s, line);
	s = SliceLeft(s, ndigits);

	n = SliceWriteCString(s, "Assertion `");
	s = SliceLeft(s, n);

	n = SliceWriteCString(s, expr);
	s = SliceLeft(s, n);

	n = SliceWriteCString(s, "' failed.");

	Write(2, s.Base, s.Len);
	Exit(1);
}


