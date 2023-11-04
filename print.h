#ifndef PRINT_H
#define PRINT_H

struct kevent ;

void Fatal(char *, int);
void FatalErr(char *, Error *);

void PrintCString(char *);
void PrintCStringLn(char *);
void PrintErr(char *, Error *);
void PrintInt(int);
void PrintKevent(struct kevent *);
void PrintMsgCode(char *, int);
void PrintNewline(void);
void PrintString(String s);
void PrintStringLn(String s);

#endif /* PRINT_H */
