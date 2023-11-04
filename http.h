#ifndef HTTP_H
#define HTTP_H

typedef struct {
	String Method;
	URL URL;
	String Version;
} HTTPRequest;

typedef struct {
	/* Buf points to 'ctx.ResponseBuffer.RemainingSlice()'. Used directly for responses with known sizes. */
	Slice Buf;
	uint64 Pos;

	/* Date points to array with current date in RFC822 format, which updates every second by kevent timer. */
	Slice Date;

	/* ContentLength points to stack-allocated buffer enough to hold 'Content-Length' header. */
	Slice ContentLength;

	/* Body points to stack-allocated 64 KiB buffer. Used only for (*HTTPResponse).WriteResponse() calls. */
	Slice Body;
} HTTPResponse;

typedef void(*HTTPRouter)(HTTPResponse *, HTTPRequest *);

Error	*ListenAndServe(int16 port, HTTPRouter router);

#endif /* HTTP_H */
