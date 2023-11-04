#include <time.h>

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
SlicePutCString(Slice s, char *cstr)
{
	uint64 cstrLen = CStringLength(cstr);
	uint64 toWrite = min(s.Len, cstrLen);

	CopyMemory(s.Base, cstr, toWrite);
	s.Len -= toWrite;

	return toWrite;
}


uint64
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


/* 08.10.2023 15:13:54 MSK */
uint64
SlicePutTm(Slice s, struct tm tm)
{
	char	*buf = s.Base;
	int	n = 0;

	if (tm.tm_mday < 10) {
		buf[n++] = '0';
	}
	n += SlicePutInt(SliceLeft(s, n), tm.tm_mday);
	buf[n++] = '.';

	if (tm.tm_mon + 1 < 10) {
		buf[n++] = '0';
	}
	n += SlicePutInt(SliceLeft(s, n), tm.tm_mon + 1);
	buf[n++] = '.';

	n += SlicePutInt(SliceLeft(s, n), tm.tm_year + 1900);
	buf[n++] = ' ';

	if (tm.tm_hour < 10) {
		buf[n++] = '0';
	}
	n += SlicePutInt(SliceLeft(s, n), tm.tm_hour);
	buf[n++] = ':';

	if (tm.tm_min < 10) {
		buf[n++] = '0';
	}
	n += SlicePutInt(SliceLeft(s, n), tm.tm_min);

	buf[n++] = ':';

	if (tm.tm_sec < 10) {
		buf[n++] = '0';
	}
	n += SlicePutInt(SliceLeft(s, n), tm.tm_sec);
	buf[n++] = ' ';

	buf[n++] = 'M';
	buf[n++] = 'S';
	buf[n++] = 'K';

	return n;
}


/* Sat, 04 Nov 2023 17:47:03 +0300 */
uint64
SlicePutTmRFC822(Slice s, struct tm tm)
{
	char	*buf = s.Base;
	int	n = 0;

	char	*wdays[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	char	*months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	n += SlicePutCString(s, wdays[tm.tm_wday]);
	buf[n++] = ',';
	buf[n++] = ' ';

	if (tm.tm_mday < 10) {
		buf[n++] = '0';
	}
	n += SlicePutInt(SliceLeft(s, n), tm.tm_mday);
	buf[n++] = ' ';

	n += SlicePutCString(SliceLeft(s, n), months[tm.tm_mon]);
	buf[n++] = ' ';

	n += SlicePutInt(SliceLeft(s, n), tm.tm_year + 1900);
	buf[n++] = ' ';

	if (tm.tm_hour < 10) {
		buf[n++] = '0';
	}
	n += SlicePutInt(SliceLeft(s, n), tm.tm_hour);
	buf[n++] = ':';

	if (tm.tm_min < 10) {
		buf[n++] = '0';
	}
	n += SlicePutInt(SliceLeft(s, n), tm.tm_min);

	buf[n++] = ':';

	if (tm.tm_sec < 10) {
		buf[n++] = '0';
	}
	n += SlicePutInt(SliceLeft(s, n), tm.tm_sec);
	buf[n++] = ' ';

	buf[n++] = '+';
	buf[n++] = '0';
	buf[n++] = '3';
	buf[n++] = '0';
	buf[n++] = '0';

	return n;
}


