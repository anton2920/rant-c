#include <sys/fcntl.h>
#include <sys/mman.h>

#include "u.h"
#include "builtin.h"

#include "assert.h"
#include "buffer.h"
#include "error.h"
#include "slice.h"
#include "syscall.h"

CircularBuffer
NewCircularBuffer(uint64 size, error *perr)
{
	CircularBuffer cb;
	char	*buffer;
	error err;
	int	fd;

	assert(size % 4096 == 0);

	cb.Buf = nil;
	cb.Len = cb.Head = cb.Tail = 0;

	fd = ShmOpen2(SHM_ANON, O_RDWR, 0, 0, nil, &err);
	if (err != nil) {
		ErrorSet(perr, err);
		return cb;
	}

	if ((err = Ftruncate(fd, size)) != nil) {
		ErrorSet(perr, err);
		return cb;
	}

	buffer = Mmap(nil, 2 * size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0, &err);
	if (err != nil) {
		ErrorSet(perr, err);
		return cb;
	}

	Mmap(buffer, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0, &err);
	if (err != nil) {
		ErrorSet(perr, err);
		return cb;
	}

	Mmap(buffer + size, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0, &err);
	if (err != nil) {
		ErrorSet(perr, err);
		return cb;
	}

	cb.Buf = buffer;
	cb.Len = 2 * size;

	/* NOTE(anton2920): sanity checks. */
	cb.Buf[0] = '\0';
	cb.Buf[size-1] = '\0';
	cb.Buf[size] = '\0';
	cb.Buf[2*size - 1] = '\0';

	ErrorSet(perr, nil);
	return cb;
}


void
Consume(CircularBuffer *cb, uint64 n)
{
	cb->Head += n;
	if (cb->Head > cb->Len / 2) {
		cb->Head -= cb->Len / 2;
		cb->Tail -= cb->Len / 2;
	}
}


void
Produce(CircularBuffer *cb, uint64 n)
{
	cb->Tail += n;
}


slice
RemainingSlice(CircularBuffer *cb)
{
	return SliceLeftRight(UnsafeSlice(cb->Buf, cb->Len), cb->Tail, cb->Head + cb->Len / 2);
}


uint64
RemainingSpace(CircularBuffer *cb)
{
	return cb->Len / 2 - (cb->Tail - cb->Head);
}


void
Reset(CircularBuffer *cb)
{
	cb->Head = cb->Tail = 0;
}


uint64
UnconsumedLen(CircularBuffer *cb)
{
	return cb->Tail - cb->Head;
}


slice
UnconsumedSlice(CircularBuffer *cb)
{
	return UnsafeSlice(&cb->Buf[cb->Head], UnconsumedLen(cb));
}


string
UnconsumedString(CircularBuffer *cb)
{
	return UnsafeString((byte * ) & cb->Buf[cb->Head], UnconsumedLen(cb));
}


