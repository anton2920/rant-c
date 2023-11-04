#include <sys/mman.h>

#include "u.h"
#include "slice.h"
#include "string.h"

#include "assert.h"
#include "error.h"
#include "pool.h"
#include "print.h"
#include "syscall.h"

struct Pool {
	Slice Items;
	NewPoolItemFunc New;
};

Pool *
NewPool(NewPoolItemFunc new, Error **err)
{
	uint64 size = 1024 * sizeof(void * );
	void * *stack;
	Pool * p;

	CheckOptionalError(err);
	assert(size % 4096 == 0);

	if ((stack = Mmap(nil, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_STACK | MAP_ANON, -1, 0)) == MAP_FAILED) {
		SetOptionalError(err, "failed to mmap stack memory region: ", SyscallErrno);
		return nil;
	}

	/* NOTE(anton2920): first item in items is Pool itself. */
	p = (Pool * )((char *)stack + size - sizeof(Pool));
	p->Items.Base = p;
	p->Items.Len = 0;
	p->Items.Cap = size - sizeof(Pool);
	p->New = new;

	return p;
}


Error *
PoolFree(Pool *p)
{
	if (Munmap((char *) p->Items.Base - p->Items.Cap, p->Items.Cap) < 0) {
		return NewError("failed to unmap memory region: ", SyscallErrno);
	}
	return nil;
}


void *
PoolGet(Pool *p)
{
	void * item;
	void * *buf;

	if (p->Items.Len > 0) {
		buf = p->Items.Base;
		item = buf[-p->Items.Len];
		p->Items.Len--;
	} else {
		item = p->New();
	}

	return item;
}


void
PoolPut(Pool *p, void *item)
{
	void * *buf = p->Items.Base;
	buf[--p->Items.Len] = item;
}


