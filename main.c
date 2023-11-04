#include "u.h"
#include "slice.h"
#include "string.h"
#include "url.h"

#include "error.h"
#include "http.h"
#include "print.h"
#include "syscall.h"

void
Router(HTTPResponse *w, HTTPRequest *r)
{
	(void)w;
	(void)r;
}


void
_start(void)
{
	const int16 port = 7070;
	Error	 * err;

	PrintCString("Listening on 0.0.0.0:7070...\n");
	if ((err = ListenAndServe(port, Router)) != nil) {
		FatalErr("Failed to start HTTP server: ", err);
	}
	Exit(0);
}


