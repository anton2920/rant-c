#include <sys/syscall.h>

#include "u.h"
#include "syscall.h"

int	SyscallErrno;

uintptr
Syscall6(int trap, uintptr a1, uintptr a2, uintptr a3, uintptr a4, uintptr a5, uintptr a6)
{
	uintptr ret;

	__asm__ __volatile__ (
	    "movq %5, %%r10\n\t"
	    "movq %6, %%r8\n\t"
	    "movq %7, %%r9\n\t"
	    "syscall\n\t"
	    "jnc Syscall6Noerror\n\t"
	    "negq %%rax\n\t"
	    "movq %%rax, SyscallErrno\n\t"
	    "movq $-1, %%rax\n\t"
	    "Syscall6Noerror:\n\t"
:
	    "=a" (ret)
:
	    "a" (trap), "D" (a1), "S" (a2), "d" (a3), "m" (a4), "m" (a5), "m" (a6)
:
	    "r10", "r8", "r9"
	    );

	return ret;
}


uintptr
Syscall(int trap, uintptr a1, uintptr a2, uintptr a3)
{
	uintptr ret;

	__asm__ __volatile__ (
	    "syscall\n\t"
	    "jnc SyscallNoerror\n\t"
	    "negq %%rax\n\t"
	    "movq %%rax, SyscallErrno\n\t"
	    "movq $-1, %%rax\n\t"
	    "SyscallNoerror:\n\t"
:
	    "=a" (ret)
:
	    "a" (trap), "D" (a1), "S" (a2), "d" (a3)
	    );

	return ret;
}


int
Accept(int s, struct sockaddr *addr, int addrlen)
{
	return Syscall(SYS_accept, s, (uintptr)addr, addrlen);
}


int
Bind(int s, struct sockaddr *addr, int addrlen)
{
	return Syscall(SYS_bind, s, (uintptr)addr, addrlen);
}


int
ClockGettime(int clockID, struct timespec *tp)
{
	return Syscall(SYS_clock_gettime, clockID, (uintptr)tp, 0);
}


int
Close(int fd)
{
	return Syscall(SYS_close, fd, 0, 0);
}


void
Exit(int code)
{
	Syscall(SYS_exit, code, 0, 0);
}


int
Ftruncate(int fd, uint64 size)
{
	return Syscall(SYS_ftruncate, fd, size, 0);
}


int
Kevent(int kq, struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, struct timespec *timeout)
{
	return Syscall6(SYS_kevent, kq, (uintptr)changelist, nchanges, (uintptr)eventlist, nevents, (uintptr)timeout);
}


int
Kqueue(void)
{
	return Syscall(SYS_kqueue, 0, 0, 0);
}


int
Listen(int s, int backlog)
{
	return Syscall(SYS_listen, s, backlog, 0);
}


void *
Mmap(void *addr, uint64 len, int prot, int flags, int fd, int64 offset)
{
	return (void * )Syscall6(SYS_mmap, (uintptr)addr, len, prot, flags, fd, offset);
}


int
Munmap(void *addr, uint64 len)
{
	return Syscall(SYS_munmap, (uintptr)addr, len, 0);
}


int
ShmOpen2(char *path, int flags, uint16 mode, int shmflags, char *name)
{
	return Syscall6(SYS_shm_open2, (uintptr)path, flags, mode, shmflags, (uintptr)name, 0);
}


int
Shutdown(int s, int how)
{
	return Syscall(SYS_shutdown, s, how, 0);
}


int
Socket(int domain, int typ, int protocol)
{
	return Syscall(SYS_socket, domain, typ, protocol);
}


int64
Write(int fd, void *buf, uint64 nbytes)
{
	return Syscall(SYS_write, fd, (uintptr) buf, nbytes);
}


