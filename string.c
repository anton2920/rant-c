#include "u.h"
#include "string.h"

#include "syscall.h"
#include "utils.h"

String
StringFrom(char *buf, uint64 len)
{
	String s;
	s.Base = buf;
	s.Len = len;

	return s;
}


String
StringFromCString(char *cstr)
{
	return StringFrom(cstr, CStringLength(cstr));
}


