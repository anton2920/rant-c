#include "u.h"
#include "builtin.h"

#include "arena.h"
#include "print.h"
#include "syscall.h"

#include <sys/mman.h>

const uint64 PageSize = 4096;

Arena GlobalArena;

uint64
ArenaSizeRoundUp(uint64 size)
{
	return size + (size & (PageSize - 1));
}


error
ExtendArenaBy(Arena a, uint64 size)
{
	void * addr;
	error err;
	int	flags;

	if (a.Base == nil) {
		addr = nil;
		flags = 0;
	} else {
		addr = (char *)a.Base + a.Allocated;
		flags = MAP_FIXED | MAP_EXCL;
	}
	size = max(PageSize, ArenaSizeRoundUp(size));

	addr = Mmap(addr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | flags, 0, -1, &err);
	if (err != nil) {
		return err;
	}
	if (a.Base == nil) {
		a.Base = addr;
	}
	a.Allocated += size;

	return nil;
}


static void *
AllocateFrom(Arena a, uint64 size)
{
	char	*start;
	error err;

	start = (char *)a.Base + a.InUse;
	if (a.InUse + size > a.Allocated) {
		if ((err = ExtendArenaBy(a, size)) != nil) {
			FatalError("Failed to initialize new arena:", err);
		}
	}
	a.InUse += size;

	return start;
}


void *
Allocate(uint64 size)
{
	return AllocateFrom(GlobalArena, size);
}


string
slicebytetostring(slice s)
{
	string ret;

	ret.base = Allocate(s.len);
	ret.len = s.len;

	copy(Slice(ret), s);
	return ret;
}


