#ifndef PRINT_H
#define PRINT_H

struct kevent ;

void Fatal(char *, int);
void FatalErr(Error *);

void PrintCString(char *);
void PrintKevent(struct kevent *);
void PrintMsgCode(char *, int);
void PrintNewline(void);
void PrintPositiveInt(int);
void PrintString(String s);

#endif /* PRINT_H */
