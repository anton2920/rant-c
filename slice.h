#ifndef SLICE_H
#define SLICE_H

typedef struct {
	void *Base;
	uint64 Len;
	uint64 Cap;
} Slice;

Slice SliceFrom(void *, uint64);

Slice SliceLeft(Slice, uint64);
Slice SliceRight(Slice, uint64);
Slice SliceLeftRight(Slice, uint64, uint64);

uint64 SliceWriteCString(Slice, char *);

int	SlicePutInt(Slice, int);

#endif /* SLICE_H */
