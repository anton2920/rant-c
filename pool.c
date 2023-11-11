#include "u.h"
#include "builtin.h"

#include "assert.h"
#include "error.h"
#include "pool.h"
#include "print.h"
#include "syscall.h"

#include <sys/mman.h>

struct Pool {
	void *Top;
	uint64 Nitems;
	NewPoolItemFunc New;
};

Pool *
NewPool(NewPoolItemFunc new, error *e)
{
	uint64 size = 1024 * sizeof(void * );
	void * *stack;
	error err;
	Pool * p;

	stack = Mmap(nil, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_STACK | MAP_ANON, -1, 0, &err);
	if (err != nil) {
		*e = err;
		return nil;
	}

	/* NOTE(anton2920): first item in items is Pool itself. */
	p = (Pool * )((char *)stack + size - sizeof(Pool));
	p->Top = p;
	p->Nitems = 0;
	p->New = new;

	return p;
}


void *
PoolGet(Pool *p)
{
	void * item;
	void * *buf;

	if (p->Nitems > 0) {
		buf = p->Top;
		item = buf[-p->Nitems];
		--p->Nitems;
	} else {
		item = p->New();
	}

	return item;
}


void
PoolPut(Pool *p, void *item)
{
	void * *buf = p->Top;
	++p->Nitems;
	buf[-p->Nitems] = item;
}


