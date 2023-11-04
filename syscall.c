#include "types.h"
#include "syscall.h"

#include <sys/syscall.h>

uintptr
Syscall(int trap, uintptr a1, uintptr a2, uintptr a3)
{
	uintptr ret;

	__asm__ __volatile__ (
	    "syscall"
:
	    "=a" (ret)
:
	    "a" (trap), "D" (a1), "S" (a2), "d" (a3)
	    );

	return ret;
}


void
Exit(int code)
{
	Syscall(SYS_exit, code, 0, 0);
}


int64
Write(int fd, void *buf, uint64 nbytes)
{
	return Syscall(SYS_write, fd, (uintptr) buf, nbytes);
}


