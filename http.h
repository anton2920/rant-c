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
	int	i;
} HTTPResponse;

typedef void(*HTTPRouter)(HTTPResponse *, HTTPRequest *);

error ListenAndServe(int16 port, HTTPRouter router);

#endif /* HTTP_H */
