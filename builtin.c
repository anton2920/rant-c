#include "u.h"
#include "builtin.h"

uint64
CStringLength(char *s)
{
	char	*saved = s;

	while (*s++)
		;

	return s - saved - 1;
}


void *
memcpy(void *_dst, const void *_src, uint64 size)
{
	const char * src = _src;
	char	*dst = _dst;

	while (size--) {
		*dst++ = *src++;
	}

	return _dst;
}


slice
UnsafeSlice(void *buf, uint64 len)
{
	slice s;

	s.base = buf;
	s.len = s.cap = len;

	return s;
}


slice
SliceLeft(slice s, uint64 lbytes)
{
	slice ret;

	ret.base = (char *)s.base + lbytes;
	ret.len = s.len - lbytes;
	ret.cap = s.cap - lbytes;

	return ret;
}


slice
SliceRight(slice s, uint64 rbytes)
{
	slice ret;

	ret.base = s.base;
	ret.len = rbytes;
	ret.cap = s.cap;

	return ret;
}


slice
SliceLeftRight(slice s, uint64 lbytes, uint64 rbytes)
{
	slice ret;

	ret = SliceLeft(s, lbytes);
	ret = SliceRight(s, rbytes);

	return ret;
}


slice
Slice(string s)
{
	slice ret;

	ret.base = s.base;
	ret.len = ret.cap = s.len;

	return ret;
}


string
UnsafeString(byte *buf, uint64 len)
{
	string s;

	s.base = buf;
	s.len = len;

	return s;
}


string
UnsafeCString(char *cstr)
{
	return UnsafeString((byte * )cstr, CStringLength(cstr));
}


string
String(slice s)
{
	string ret;

	ret.base = s.base;
	ret.len = s.len;

	return ret;
}


uint64
copy(slice dst, slice src)
{
	uint64 toCopy = min(dst.len, src.len);

	memcpy(dst.base, src.base, toCopy);
	return toCopy;
}


