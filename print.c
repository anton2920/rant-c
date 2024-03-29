#include "u.h"
#include "builtin.h"

#include "error.h"
#include "http.h"
#include "print.h"
#include "slice.h"
#include "syscall.h"

#include <sys/event.h>

uint64 CStringLength(char *);

void
Fatal(char *msg)
{
	PrintCString(msg);
	Exit(1);
}


void
FatalError(char *msg, error err)
{
	PrintError(msg, err);
	Exit(1);
}


static void
PrintNewline(void)
{
	Write(2, "\n", 1, nil);
}


static void
Print(char *cstr)
{
	Write(2, cstr, CStringLength(cstr), nil);
}


void
PrintCString(char *cstr)
{
	Print(cstr);
	PrintNewline();
}


void PrintError(char *msg, error err)
{
	Print(msg);
	Print(" ");
	if (err != nil) {
		PrintString(err->Error(err));
	} else {
		PrintNewline();
	}
}


void
PrintInt(int x)
{
	char	buffer[20];
	int	n;
	slice s;

	s = UnsafeSlice(buffer, sizeof(buffer));
	n = SlicePutInt(s, x);
	PrintString(String(SliceRight(s, n)));
}


void
PrintKevent(struct kevent *e)
{
	char	buffer[100];
	int	n = 0;
	slice s;

	s = UnsafeSlice(buffer, sizeof(buffer));

	n += SlicePutCString(SliceLeft(s, n), "EVENT: ");
	n += SlicePutInt(SliceLeft(s, n), e->ident);
	n += SlicePutCString(SliceLeft(s, n), " ");
	n += SlicePutInt(SliceLeft(s, n), e->filter);
	n += SlicePutCString(SliceLeft(s, n), " ");
	n += SlicePutInt(SliceLeft(s, n), e->flags & 0xF);
	n += SlicePutCString(SliceLeft(s, n), " ");
	n += SlicePutInt(SliceLeft(s, n), e->data);

	PrintString(String(SliceRight(s, n)));
}


void
PrintMsgCode(char *msg, int code)
{
	Print(msg);
	Print(" ");
	PrintInt(code);
}


void
PrintRequest(void *_r)
{
	HTTPRequest * r = _r;
	char	buffer[100];
	int	n = 0;
	slice s;

	s = UnsafeSlice(buffer, sizeof(buffer));

	n += SlicePutCString(SliceLeft(s, n), "Executed: ");
	n += SlicePutString(SliceLeft(s, n), r->Method);
	n += SlicePutCString(SliceLeft(s, n), " ");
	n += SlicePutString(SliceLeft(s, n), r->URL.Path);
	n += SlicePutCString(SliceLeft(s, n), " ");
	n += SlicePutString(SliceLeft(s, n), r->URL.Query);

	PrintString(String(SliceRight(s, n)));
}


void
PrintString(string s)
{
	Write(2, s.base, s.len, nil);
	PrintNewline();
}


