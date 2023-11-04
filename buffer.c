#include <sys/fcntl.h>
#include <sys/mman.h>

#include "u.h"
#include "slice.h"
#include "string.h"

#include "assert.h"
#include "error.h"
#include "syscall.h"

#include "buffer.h"

CircularBuffer
NewCircularBuffer(uint64 size, Error **err)
{
	CircularBuffer cb;
	char	*buffer;
	int	fd;

	CheckOptionalError(err);
	assert(size % 4096 == 0);

	cb.Buf = nil;
	cb.Len = cb.Head = cb.Tail = 0;

	if ((fd = ShmOpen2(SHM_ANON, O_RDWR, 0, 0, nil)) < 0) {
		SetOptionalError(err, "failed to open shared memory region: ", SyscallErrno);
		return cb;
	}

	if (Ftruncate(fd, size) < 0) {
		SetOptionalError(err, "failed to adjust size of shared memory region: ",  SyscallErrno);
		return cb;
	}

	if ((buffer = Mmap(nil, 2 * size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0)) == MAP_FAILED) {
		SetOptionalError(err, "failed to adjust size of shared memory region",  SyscallErrno);
		return cb;
	}

	if (Mmap(buffer, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0) == MAP_FAILED) {
		SetOptionalError(err, "failed to map first view of buffer: ",  SyscallErrno);
		return cb;
	}

	if (Mmap(buffer + size, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0) == MAP_FAILED) {
		SetOptionalError(err, "failed to adjust size of shared memory region: ",  SyscallErrno);
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


