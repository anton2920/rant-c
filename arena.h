#ifndef ARENA_H
#define ARENA_H

#define newobject(typ) Allocate(sizeof(typ))

typedef struct {
	void *Base;
	uint64 InUse;
	uint64 Allocated;
} Arena;

void *Allocate(uint64);

string slicebytetostring(slice);

#endif /* ARENA_H */
