#include "u.h"
#include "builtin.h"

#include "arena.h"
#include "assert.h"
#include "atomic.h"
#include "buffer.h"
#include "error.h"
#include "http.h"
#include "pool.h"
#include "print.h"
#include "slice.h"
#include "syscall.h"
#include "time.h"

#include <netinet/in.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>

typedef enum {
	HTTP_STATE_UNKNOWN,

	HTTP_STATE_METHOD,
	HTTP_STATE_URI,
	HTTP_STATE_VERSION,
	HTTP_STATE_HEADER,

	HTTP_STATE_DONE
} HTTPRequestParserState;

typedef struct {
	HTTPRequestParserState State;
	HTTPRequest Request;
} HTTPRequestParser;

typedef struct {
	CircularBuffer RequestBuffer;

	HTTPRequestParser Parser;

	uintptr Check;
} HTTPContext;

int16
SwapBytesInPort(int16 port)
{
	return (port << 8) | ((port & 0xFF00) >> 8);
}


HTTPContext *
NewHTTPContext()
{
	HTTPContext * c = newobject(HTTPContext);
	error err;

	if (c == nil) {
		return nil;
	}

	c->RequestBuffer = NewCircularBuffer(2 * 4096, &err);
	if (err != nil) {
		PrintError("ERROR: failed to create request buffer:", err);
		return nil;
	}

	return c;
}


void
HTTPWorker(int l, HTTPRouter router)
{
	struct kevent chlist[2], events[256], *e;
	int	kq, nevents, i, c;
	HTTPContext * ctx;
	struct timespec tp;
	char	dateBuf[31];
	Pool * ctxPool;
	uintptr check;
	slice date;
	error err;

	kq = Kqueue(&err);
	if (err != nil) {
		FatalError("Failed to open kqueue:", err);
	}

	EV_SET(&chlist[0], l, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, 0);
	EV_SET(&chlist[1], 1, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, 1, 0);
	Kevent(kq, chlist, 2, nil, 0, nil, &err);
	if (err != nil) {
		FatalError("Failed to add event for listener socket:", err);
	}

	if ((err = ClockGettime(CLOCK_REALTIME, &tp)) != nil) {
		FatalError("Failed to get current walltime:", err);
	}
	tp.tv_nsec = 0;
	date = UnsafeSlice(dateBuf, sizeof(dateBuf));

	ctxPool = NewPool((NewPoolItemFunc)NewHTTPContext, &err);
	if (err != nil) {
		FatalError("Failed to create HTTP context pool: ", err);
	}

	while (1) {
		nevents = Kevent(kq, nil, 0, events, sizeof(events) / sizeof(events[0]), nil, &err);
		if (err != nil) {
			int	code = ((E *)err)->Code;
			if (code == EINTR) {
				continue;
			}
			PrintMsgCode("ERROR: failed to get requested kernel events:", code);
		}

		for (i = 0; i < nevents; ++i) {
			e = &events[i];
			c = e->ident;

			/* PrintKevent(e); */

			if (c == l) {
				void * udata;

				c = Accept(l, nil, 0, &err);
				if (err != nil) {
					PrintError("ERROR: failed to accept connection:", err);
					continue;
				}

				ctx = PoolGet(ctxPool);
				if (ctx == nil) {
					Fatal("Failed to acquire new context");
				}

				udata = (void * )((uintptr)ctx | ctx->Check);
				EV_SET(&chlist[0], c, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, udata);
				EV_SET(&chlist[1], c, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, udata);
				Kevent(kq, chlist, 2, nil, 0, nil, &err);
				if (err != nil) {
					PrintError("ERROR: failed to add new events to kqueue:", err);
					goto closeConnection;
				}
				continue;
			} else if (c == 1) {
				tp.tv_sec += e->data;
				SlicePutTmRFC822(date, TimeToTm(tp.tv_sec));
				/* PrintString(UnsafeString(date.base, date.len)); */
				continue;
			}

			check = (uintptr)e->udata & 0x1;
			ctx = (void * )((uintptr)e->udata - check);
			if (check != ctx->Check) {
				continue;
			}

			switch (e->filter) {
			case EVFILT_READ:
				break;
			case EVFILT_WRITE:
				Write(c, "Hello, world!\n", sizeof("Hello, world!\n") - 1, &err);
				if (err != nil) {
					PrintError("ERROR: failed to write hello, world!:", err);
				}
				goto closeConnection;
				break;
			}

closeConnection:
			ctx->Check = 1 - ctx->Check;
			PoolPut(ctxPool, ctx);
			Shutdown(c, SHUT_WR);
			Close(c);
		}
	}

	(void)router;
}


error
ListenAndServe(int16 port, HTTPRouter router)
{
	struct sockaddr_in addr;
	int	enable = 1;
	error err;
	int	l;

	l = Socket(PF_INET, SOCK_STREAM, 0, &err);
	if (err != nil) {
		return err;
	}

	if ((err = Setsockopt(l, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable))) != nil) {
		return err;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = SwapBytesInPort(port);
	if ((err = Bind(l, (struct sockaddr *) & addr, sizeof(addr))) != nil) {
		return err;
	}

	if ((err = Listen(l, 128)) != nil) {
		return err;
	}

	/* TODO(anton2920): add more threads. */
	HTTPWorker(l, router);

	return nil;
}


