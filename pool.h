#ifndef POOL_H
#define POOL_H

typedef void *(*NewPoolItemFunc)(void);

typedef struct Pool Pool;

Pool *NewPool(NewPoolItemFunc, error *);
error PoolFree(Pool *);

void *PoolGet(Pool *);
void PoolPut(Pool *, void *);

#endif /* POOL_H */
