#ifndef SYSCALL_H
#define SYSCALL_H

void Exit(int);
int64 Write(int fd, void *buf, uint64 nbytes);

#endif /* SYSCALL_H */
