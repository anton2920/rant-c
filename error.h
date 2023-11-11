#ifndef ERROR_H
#define ERROR_H

#define ErrorSet(perr, err) \
	do { \
		if (perr != nil) { \
			*perr = err; \
		} \
	} while(0)

typedef struct {
	string (*Error)(error);
	string Message;
	int	Code;
} E;

error Error(char *);
error ErrorWithCode(char *, int);
error SyscallError(char *, uintptr);

#endif /* ERROR_H */
