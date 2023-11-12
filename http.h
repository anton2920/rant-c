#ifndef HTTP_H
#define HTTP_H

typedef struct {
	string Path;
	string Query;
} URL;

typedef struct {
	string Method;
	URL URL;
	string Version;
} HTTPRequest;

typedef struct {
	/* Holds pointers to response. Data must live long enough. */
	slice *Iovs;

	/* ContentLength is calculated with each WriteBodyNoCopy call. */
	uint64 ContentLength;

	/* ContentLengthBuf points to stack-allocated buffer enough to hold 'Content-Length' header. */
	slice ContentLengthBuf;

	/* DateBuf points to array with current date in RFC822 format, which updates every second by kevent timer. */
	slice DateBuf;
} HTTPResponse;

typedef void(*HTTPRouter)(HTTPResponse *, HTTPRequest *);

error ListenAndServe(int16 port, HTTPRouter router);

void WriteResponseNoCopy(HTTPResponse *, string, slice);

#endif /* HTTP_H */
