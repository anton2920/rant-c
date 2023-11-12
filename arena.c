#include "u.h"
#include "builtin.h"

#include "assert.h"
#include "print.h"
#include "syscall.h"

#include <sys/mman.h>

typedef struct {
	void *Base;
	uint64 InUse;
	uint64 Allocated;
} Arena;

const uint64 PageSize = 4096;

Arena GlobalArena;

uint64
ArenaSizeRoundUp(uint64 size)
{
	return size + (size & (PageSize - 1));
}


error
ExtendArenaBy(Arena *a, uint64 size)
{
	void * addr;
	error err;
	int	flags;

	if (a->Base == nil) {
		addr = nil;
		flags = 0;
	} else {
		addr = (char *)a->Base + a->Allocated;
		flags = MAP_FIXED | MAP_EXCL;
	}
	size = max(PageSize, ArenaSizeRoundUp(size));

	addr = Mmap(addr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | flags, -1, 0, &err);
	if (err != nil) {
		return err;
	}
	if (a->Base == nil) {
		a->Base = addr;
	}
	a->Allocated += size;

	return nil;
}


static void *
AllocateFrom(Arena *a, uint64 size)
{
	char	*start;
	error err;

	if (a->InUse + size > a->Allocated) {
		if ((err = ExtendArenaBy(a, size)) != nil) {
			FatalError("Failed to initialize new arena:", err);
		}
	}
	start = (char *)a->Base + a->InUse;
	a->InUse += size;

	return start;
}


void *
Allocate(uint64 size)
{
	return AllocateFrom(&GlobalArena, size);
}





