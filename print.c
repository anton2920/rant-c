#include <sys/event.h>

#include "u.h"
#include "slice.h"
#include "string.h"

#include "error.h"
#include "print.h"
#include "syscall.h"
#include "utils.h"

void
Fatal(char *msg, int code)
{
	PrintMsgCode(msg, code);
	Exit(1);
}


void
FatalErr(char *msg, Error *err)
{
	PrintErr(msg, err);
	Exit(1);
}


void
PrintCString(char *cstr)
{
	Write(2, cstr, CStringLength(cstr));
}


void
PrintCStringLn(char *cstr)
{
	PrintCString(cstr);
	PrintNewline();
}


void PrintErr(char *msg, Error *err)
{
	PrintCString(msg);
	PrintString(err->Message);
	PrintInt(err->Code);
	PrintNewline();
}


void
PrintInt(int x)
{
	char	buffer[20];
	int	ndigits;
	Slice s;

	s = SliceFrom(buffer, sizeof(buffer));
	ndigits = SlicePutInt(s, x);
	Write(2, s.Base, ndigits);
}


void
PrintKevent(struct kevent *e)
{
	char	buffer[100];
	int	n = 0;
	Slice s;

	s = SliceFrom(buffer, sizeof(buffer));

	n += SlicePutCString(SliceLeft(s, n), "EVENT: ");
	n += SlicePutInt(SliceLeft(s, n), e->ident);
	n += SlicePutCString(SliceLeft(s, n), " ");
	n += SlicePutInt(SliceLeft(s, n), e->filter);
	n += SlicePutCString(SliceLeft(s, n), " ");
	n += SlicePutInt(SliceLeft(s, n), e->flags & 0xF);
	n += SlicePutCString(SliceLeft(s, n), " ");
	n += SlicePutInt(SliceLeft(s, n), e->data);

	PrintString(StringFrom(s.Base, n));
	PrintNewline();
}


void
PrintMsgCode(char *msg, int code)
{
	PrintCString(msg);
	PrintInt(code);
	PrintNewline();
}


void
PrintNewline(void)
{
	Write(2, "\n", 1);
}


void
PrintString(String s)
{
	Write(2, s.Base, s.Len);
}


void
PrintStringLn(String s)
{
	PrintString(s);
	PrintNewline();
}


