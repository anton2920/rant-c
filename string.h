#ifndef STRING_H
#define STRING_H

typedef struct {
	char	*Base;
	uint64 Len;
} String;

String StringFromCString(char *);

void PrintString(String);

#endif /* STRING_H */
