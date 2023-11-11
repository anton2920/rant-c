#ifndef PRINT_H
#define PRINT_H

struct kevent ;

void Fatal(char *);
void FatalError(char *, error);

void PrintCString(char *);
void PrintError(char *, error);
void PrintKevent(struct kevent *);
void PrintMsgCode(char *, int);
void PrintString(string s);

#endif /* PRINT_H */
