#ifndef IOVEC_H
#define IOVEC_H

typedef struct iovec Iovec;

Iovec IovecForByteSlice(slice);
Iovec IovecForCString(char *);
Iovec IovecForString(string);

#endif /* IOVEC_H */
