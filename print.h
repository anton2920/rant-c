#ifndef PRINT_H
#define PRINT_H

struct kevent ;
struct HTTPRequest ;

void Fatal(char *);
void FatalError(char *, error);

void PrintCString(char *);
void PrintError(char *, error);
void PrintInt(int x);
void PrintKevent(struct kevent *);
void PrintMsgCode(char *, int);
void PrintRequest(void *);
void PrintString(string s);

#endif /* PRINT_H */
