#ifndef SYSCALL_H
#define SYSCALL_H

struct kevent ;
struct sockaddr ;
struct timespec ;

int	Accept(int, struct sockaddr *, int);
int	Bind(int, struct sockaddr *, int);
int	ClockGettime(int clockID, struct timespec *tp);
int	Close(int);
void Exit(int);
int	Kevent(int, struct kevent *, int, struct kevent *, int, struct timespec *);
int	Kqueue(void);
int	Listen(int, int);
int	Socket(int, int, int);
int64 Write(int, void *, uint64);

#endif /* SYSCALL_H */
