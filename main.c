#include "types.h"
#include "syscall.h"

#define PrintString(str) Write(1, str, sizeof(str)-1)

void
_start()
{
	PrintString("Hello, world!\n");
	Exit(0);
}


