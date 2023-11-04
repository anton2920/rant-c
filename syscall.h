#ifndef SYSCALL_H
#define SYSCALL_H

struct kevent ;
struct sockaddr ;
struct timespec ;

extern int	SyscallErrno;

int	Accept(int, struct sockaddr *, int);
int	Bind(int, struct sockaddr *, int);
int	ClockGettime(int, struct timespec *);
int	Close(int);
void Exit(int);
int	Ftruncate(int, uint64);
int	Kevent(int, struct kevent *, int, struct kevent *, int, struct timespec *);
int	Kqueue(void);
int	Listen(int, int);
void *Mmap(void *, uint64, int, int, int, int64);
int	Munmap(void *, uint64);
int	ShmOpen2(char *, int, uint16, int, char *);
int	Shutdown(int, int);
int	Socket(int, int, int);
int64 Write(int, void *, uint64);

#endif /* SYSCALL_H */
