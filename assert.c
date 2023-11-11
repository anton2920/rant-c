#include "u.h"
#include "builtin.h"

#include "print.h"
#include "slice.h"
#include "syscall.h"

void
_assert_fail(char *expr, char *file, int line)
{
	char	buffer[1024];
	uint64 n = 0;
	slice s;

	/* file:line: Assertion `expr' failed. */
	s = UnsafeSlice(buffer, sizeof(buffer));

	n += SlicePutCString(SliceLeft(s, n), file);
	n += SlicePutCString(SliceLeft(s, n), ":");
	n += SlicePutInt(SliceLeft(s, n), line);
	n += SlicePutCString(SliceLeft(s, n), "Assertion `");
	n += SlicePutCString(SliceLeft(s, n), expr);
	n += SlicePutCString(SliceLeft(s, n), "' failed.");

	PrintString(String(SliceRight(s, n)));
	Exit(1);
}


