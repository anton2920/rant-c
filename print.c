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
FatalErr(Error *e)
{
	PrintCString("Fatal: ");
	PrintString(e->Message);
	PrintPositiveInt(e->Code);
	PrintNewline();
	Exit(1);
}


void
PrintCString(char *cstr)
{
	Write(2, cstr, CStringLength(cstr));
}


void
PrintKevent(struct kevent *e)
{
	PrintCString("EVENT: ");
	PrintPositiveInt(e->ident);
	PrintCString(" ");
	PrintPositiveInt(e->filter);
	PrintCString(" ");
	PrintPositiveInt(e->flags);
	PrintNewline();
}


void
PrintMsgCode(char *msg, int code)
{
	PrintCString(msg);
	PrintPositiveInt(code);
	PrintNewline();
}


void
PrintNewline(void)
{
	Write(2, "\n", 1);
}


void
PrintPositiveInt(int x)
{
	char	buffer[20];
	int	ndigits;
	Slice s;

	s = SliceFrom(buffer, sizeof(buffer));
	ndigits = SlicePutInt(s, x);
	Write(2, s.Base, ndigits);
}


void
PrintString(String s)
{
	Write(2, s.Base, s.Len);
}


