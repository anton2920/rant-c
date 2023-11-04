#ifndef ERROR_H
#define ERROR_H

typedef struct {
	String Message;
	int	Code;
} Error;

Error *NewError(char *msg, int code);

#endif /* ERROR_H */
