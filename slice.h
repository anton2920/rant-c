#ifndef SLICE_H
#define SLICE_H

struct tm ;

typedef struct {
	void *Base;
	uint64 Len;
	uint64 Cap;
} Slice;

Slice SliceFrom(void *, uint64);

Slice SliceLeft(Slice, uint64);
Slice SliceRight(Slice, uint64);
Slice SliceLeftRight(Slice, uint64, uint64);

uint64 SlicePutCString(Slice, char *);

uint64 SlicePutInt(Slice, int);
uint64 SlicePutTm(Slice, struct tm);
uint64 SlicePutTmRFC822(Slice, struct tm);

#endif /* SLICE_H */
