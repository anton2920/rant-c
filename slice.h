#ifndef SLICE_H
#define SLICE_H

struct tm ;

uint64 SlicePutCString(slice, char *);
uint64 SlicePutString(slice, string);
uint64 SlicePutInt(slice, int);
uint64 SlicePutTm(slice, struct tm);
uint64 SlicePutTmRFC822(slice, struct tm);

#endif /* SLICE_H */
