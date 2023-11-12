#include "u.h"
#include "builtin.h"
#include "runtime.h"

#include "error.h"
#include "http.h"
#include "print.h"
#include "syscall.h"

void
PlaintextHandler(HTTPResponse *w, HTTPRequest *r)
{
	(void)r;

	WriteResponseNoCopy(w, StringLiteral("text/plain"), Slice(StringLiteral("Hello, world!\n")));
}


void
Router(HTTPResponse *w, HTTPRequest *r)
{
	if (memequal(r->URL.Path.base, "/plaintext", sizeof("/plaintext") - 1)) {
		PlaintextHandler(w, r);
	}
}


void
_start(void)
{
	const int16 port = 7070;
	error err;

	PrintCString("Listening on 0.0.0.0:7070...");
	if ((err = ListenAndServe(port, Router)) != nil) {
		FatalError("Failed to start HTTP server:", err);
	}

	Exit(0);
}


