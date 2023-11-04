#include "u.h"
#include "slice.h"

#include "syscall.h"

void *
CopyMemory(void *_dst, void *_src, uint64 size)
{
	char	*dst = _dst, *src = _src;

	while (size--) {
		*dst++ = *src++;
	}

	return _dst;
}


uint64
CStringLength(char *s)
{
	char	*saved = s;

	while (*s++)
		;

	return s - saved;
}


/*
void *
memcpy(void *dst, const void *src, uint64 size)
{
	union {
		void *ptr;
		const void * cptr;
	} cast;

	cast.cptr = src;
	return CopyMemory(dst, cast.ptr, size);
}
*/


