#ifndef SYSCALL_H
#define SYSCALL_H

struct iov ;
struct kevent ;
struct sockaddr ;
struct timespec ;

int	Accept(int, struct sockaddr *, int, error *);
error	Bind(int, struct sockaddr *, int);
error	ClockGettime(int, struct timespec *);
error	Close(int);
void Exit(int);
int	Fork(error *);
error	Ftruncate(int, uint64);
int	Kevent(int, struct kevent *, int, struct kevent *, int, struct timespec *, error *);
int	Kqueue(error *);
error	Listen(int, int);
void *Mmap(void *, uint64, int, int, int, int64, error *);
int64 Read(int, slice, error *);
error Setsockopt(int, int, int, void *, unsigned);
int	ShmOpen2(char *, int, uint16, int, char *, error *);
error	Shutdown(int, int);
int	Socket(int, int, int, error *);
int64 Write(int, void *, uint64, error *);
int64 Writev(int, slice, error *);

#endif /* SYSCALL_H */
