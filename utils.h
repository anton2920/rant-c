#ifndef UTILS_H
#define UTILS_H

#define ArrayLength(arr) (sizeof(arr) / sizeof((arr)[0]))

void *CopyMemory(void *_dst, void *_src, uint64 size);
uint64 CStringLength(char *s);

#endif /* UTILS_H */
