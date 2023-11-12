#ifndef RUNTIME_H
#define RUNTIME_H

#define Breakpoint __asm__ __volatile__ ("int3")

#define new(typ) newobject(typ)
#define newobject(typ) Allocate(sizeof(typ))

void *Allocate(uint64);

slice growslice(void *, uint64, uint64, uint64, uint64);
string slicebytetostring(slice);

void *memcpy(void *, const void *, unsigned long);
void *memmove(void *, const void *, unsigned long);
int	memequal(void *, void *, unsigned long);

void panic(char *);

#endif /* RUNTIME_H */
