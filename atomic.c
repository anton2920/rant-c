#include "u.h"

int
AtomicAddInt32(int *addr, int delta)
{
	int	val;

	__asm__ __volatile__ (
	    "lock xaddl %1, (%2)"
:
	    "=r" (val)
:
	    "0" (delta), "r" (addr)
	    );

	return val + delta;
}


