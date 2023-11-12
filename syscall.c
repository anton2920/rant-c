#include "u.h"
#include "builtin.h"

#include "error.h"
#include "syscall.h"

#include <sys/syscall.h>

typedef struct {
	uintptr r1;
	uintptr r2;
	uintptr errno;
} SyscallResult;

SyscallResult
Syscall6(int trap, uintptr a1, uintptr a2, uintptr a3, uintptr a4, uintptr a5, uintptr a6)
{
	SyscallResult r;

	__asm__ __volatile__ (
	    "movq	%7, %%r10\n\t"
	    "movq	%8, %%r8\n\t"
	    "movq	%9, %%r9\n\t"
	    "syscall\n\t"
	    "jnc 0f\n\t"
	    "movq	$-1, %0\n\t"
	    "movq	$0, %1\n\t"
	    "movq	%%rax, %2\n\t"
	    "jmp 1f\n\t"
	    "0:\n\t"
	    "movq	%%rax, %0\n\t"
	    "movq	%%rdx, %1\n\t"
	    "movq	$0, %2\n\t"
	    "1:\n\t"
:
	    "=m" (r.r1), "=m" (r.r2), "=m" (r.errno)
:
	    "a" (trap), "D" (a1), "S" (a2), "d" (a3), "m" (a4), "m" (a5), "m" (a6)
:
	    "rcx", "r8", "r9", "r10", "r11"
	    );

	return r;
}


SyscallResult
Syscall(int trap, uintptr a1, uintptr a2, uintptr a3)
{
	SyscallResult r;

	__asm__ __volatile__ (
	    "syscall\n\t"
	    "jnc 0f\n\t"
	    "movq	$-1, %0\n\t"
	    "movq	$0, %1\n\t"
	    "movq	%%rax, %2\n\t"
	    "jmp 1f\n\t"
	    "0:\n\t"
	    "movq	%%rax, %0\n\t"
	    "movq	%%rdx, %1\n\t"
	    "movq	$0, %2\n\t"
	    "1:\n\t"
:
	    "=m" (r.r1), "=m" (r.r2), "=m" (r.errno)
:
	    "a" (trap), "D" (a1), "S" (a2), "d" (a3)
:
	    "rcx", "r8", "r9", "r10", "r11"
	    );

	return r;
}


int
Accept(int s, struct sockaddr *addr, int addrlen, error *perr)
{
	SyscallResult r = Syscall(SYS_accept, s, (uintptr)addr, addrlen);
	ErrorSet(perr, SyscallError("accept failed with code", r.errno));
	return r.r1;
}


error
Bind(int s, struct sockaddr *addr, int addrlen)
{
	SyscallResult r = Syscall(SYS_bind, s, (uintptr)addr, addrlen);
	return SyscallError("bind failed with code", r.errno);
}


error
ClockGettime(int clockID, struct timespec *tp)
{
	SyscallResult r = Syscall(SYS_clock_gettime, clockID, (uintptr)tp, 0);
	return SyscallError("clock_gettime failed with code", r.errno);
}


error
Close(int fd)
{
	SyscallResult r = Syscall(SYS_close, fd, 0, 0);
	return SyscallError("close failed with code", r.errno);
}


void
Exit(int code)
{
	Syscall(SYS_exit, code, 0, 0);
}


int
Fork(error *perr)
{
	SyscallResult r = Syscall(SYS_fork, 0, 0, 0);
	ErrorSet(perr, SyscallError("fork failed with code", r.errno));
	return r.r2;
}


error
Ftruncate(int fd, uint64 size)
{
	SyscallResult r =  Syscall(SYS_ftruncate, fd, size, 0);
	return SyscallError("ftruncate failed with code", r.errno);
}


int
Kevent(int kq, struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, struct timespec *timeout, error *perr)
{
	SyscallResult r =  Syscall6(SYS_kevent, kq, (uintptr)changelist, nchanges, (uintptr)eventlist, nevents, (uintptr)timeout);
	ErrorSet(perr, SyscallError("kevent failed with code", r.errno));
	return r.r1;
}


int
Kqueue(error *perr)
{
	SyscallResult r = Syscall(SYS_kqueue, 0, 0, 0);
	ErrorSet(perr, SyscallError("kqueue failed with code", r.errno));
	return r.r1;
}


error
Listen(int s, int backlog)
{
	SyscallResult r =  Syscall(SYS_listen, s, backlog, 0);
	return SyscallError("listen failed with code", r.errno);
}


void *
Mmap(void *addr, uint64 len, int prot, int flags, int fd, int64 offset, error *perr)
{
	SyscallResult r = Syscall6(SYS_mmap, (uintptr)addr, len, prot, flags, fd, offset);
	ErrorSet(perr, SyscallError("mmap failed with code", r.errno));
	return (void * )r.r1;
}


int64
Read(int fd, slice buf, error *perr)
{
	SyscallResult r = Syscall(SYS_read, fd, (uintptr)buf.base, buf.len);
	ErrorSet(perr, SyscallError("read failed with code", r.errno));
	return r.r1;
}


error
Setsockopt(int s, int level, int optname, void *optval, unsigned optlen)
{
	SyscallResult r = Syscall6(SYS_setsockopt, s, level, optname, (uintptr)optval, optlen, 0);
	return SyscallError("setsockopt failed with code", r.errno);
}


int
ShmOpen2(char *path, int flags, uint16 mode, int shmflags, char *name, error *perr)
{
	SyscallResult r = Syscall6(SYS_shm_open2, (uintptr)path, flags, mode, shmflags, (uintptr)name, 0);
	ErrorSet(perr, SyscallError("shm_open2 failed with code", r.errno));
	return r.r1;
}


error
Shutdown(int s, int how)
{
	SyscallResult r = Syscall(SYS_shutdown, s, how, 0);
	return SyscallError("shutdown failed with code", r.errno);
}


int
Socket(int domain, int typ, int protocol, error *perr)
{
	SyscallResult r = Syscall(SYS_socket, domain, typ, protocol);
	ErrorSet(perr, SyscallError("socket failed with code", r.errno));
	return r.r1;
}


int64
Write(int fd, void *buf, uint64 nbytes, error *perr)
{
	SyscallResult r = Syscall(SYS_write, fd, (uintptr) buf, nbytes);
	ErrorSet(perr, SyscallError("write failed with code", r.errno));
	return r.r1;
}


int64
Writev(int fd, slice iovs, error *perr)
{
	SyscallResult r = Syscall(SYS_writev, fd, (uintptr)iovs.base, iovs.len);
	ErrorSet(perr, SyscallError("writev failed with code", r.errno));
	return r.r1;
}


