#ifndef ERROR_H
#define ERROR_H

#define CheckOptionalError(err) assert(((err) != nil) && (*(err) == nil))

typedef struct {
	String Message;
	int	Code;
} Error;

Error *NewError(char *, int);

void SetOptionalError(Error **, char *, int);

#endif /* ERROR_H */
