#ifndef STRING_H
#define STRING_H

typedef struct {
	char	*Base;
	uint64 Len;
} String;

String StringFrom(char *, uint64);
String StringFromCString(char *);

#endif /* STRING_H */
