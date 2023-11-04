#ifndef POOL_H
#define POOL_H

typedef void *(*NewPoolItemFunc)();

typedef struct Pool Pool;

Pool *NewPool(NewPoolItemFunc, Error **);
Error *PoolFree(Pool *);

void *PoolGet(Pool *);
void PoolPut(Pool *, void *);

#endif /* POOL_H */
