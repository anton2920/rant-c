#include "u.h"
#include "slice.h"

#include "utils.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

Slice
SliceFrom(void *buf, uint64 size)
{
	Slice s;
	s.Base = buf;
	s.Len = s.Cap = size;
	return s;
}


Slice
SliceLeft(Slice s, uint64 lbytes)
{
	Slice ret;

	ret.Base = (char *)s.Base + lbytes;
	ret.Len = s.Len - lbytes;
	ret.Cap = s.Cap - lbytes;

	return ret;
}


Slice
SliceRight(Slice s, uint64 rbytes)
{
	Slice ret;

	ret.Len = s.Len - rbytes;

	return ret;
}


Slice
SliceLeftRight(Slice s, uint64 lbytes, uint64 rbytes)
{
	Slice ret;

	ret = SliceLeft(s, lbytes);
	ret = SliceRight(s, rbytes);

	return ret;
}


uint64
SliceWriteCString(Slice s, char *cstr)
{
	uint64 cstrLen = CStringLength(cstr);
	uint64 toWrite = min(s.Len, cstrLen);

	CopyMemory(s.Base, cstr, toWrite);
	s.Len -= toWrite;

	return toWrite;
}


int
SlicePutInt(Slice s, int x)
{
	int	ndigits = 0, rx, i = 0;
	char	*buf = s.Base;
	int	sign = x < 0;

	if (x == 0) {
		buf[0] = '0';
		return 1;
	}

	if (sign) {
		x = -x;
		buf[i++] = '-';
	}

	while (x > 0) {
		rx = (10 * rx) + (x % 10);
		x /= 10;
		++ndigits;
	}

	while (ndigits > 0) {
		buf[i++] = (rx % 10) + '0';
		rx /= 10;
		--ndigits;
	}

	return i;
}


