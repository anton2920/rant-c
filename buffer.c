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
NewCircularBuffer(uint64 size, error *e)
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
		*e = err;
		return cb;
	}

	if ((err = Ftruncate(fd, size)) != nil) {
		*e = err;
		return cb;
	}

	buffer = Mmap(nil, 2 * size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0, &err);
	if (err != nil) {
		*e = err;
		return cb;
	}

	Mmap(buffer, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0, &err);
	if (err != nil) {
		*e = err;
		return cb;
	}

	Mmap(buffer + size, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0, &err);
	if (err != nil) {
		*e = err;
		return cb;
	}

	cb.Buf = buffer;
	cb.Len = size;

	/* NOTE(anton2920): sanity checks. */
	cb.Buf[0] = '\0';
	cb.Buf[size-1] = '\0';
	cb.Buf[size] = '\0';
	cb.Buf[2*size - 1] = '\0';

	return cb;
}


