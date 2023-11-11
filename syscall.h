#ifndef SYSCALL_H
#define SYSCALL_H

struct kevent ;
struct sockaddr ;
struct timespec ;

error	Accept(int, struct sockaddr *, int);
error	Bind(int, struct sockaddr *, int);
error	ClockGettime(int, struct timespec *);
error	Close(int);
void Exit(int);
error	Ftruncate(int, uint64);
int	Kevent(int, struct kevent *, int, struct kevent *, int, struct timespec *, error *);
int	Kqueue(error *);
error	Listen(int, int);
void *Mmap(void *, uint64, int, int, int, int64, error *);
int	ShmOpen2(char *, int, uint16, int, char *, error *);
error	Shutdown(int, int);
int	Socket(int, int, int, error *);
int64 Write(int, void *buf, uint64, error *);

#endif /* SYSCALL_H */
