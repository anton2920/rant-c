#include <netinet/in.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>

#include "u.h"
#include "slice.h"
#include "string.h"
#include "url.h"

#include "atomic.h"
#include "error.h"
#include "http.h"
#include "pool.h"
#include "print.h"
#include "syscall.h"
#include "time.h"
#include "utils.h"

#include "buffer.h"

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
	CircularBuffer ResponseBuffer;

	HTTPRequestParser Parser;

	uintptr Check;
} HTTPContext;

int16
SwapBytesInPort(int16 port)
{
	return (port << 8) | ((port & 0xFF00) >> 8);
}


/* TODO(anton2920): replace with dynamic allocations. */
HTTPContext HTTPContextArena[1024];
int	HTTPContextArenaLastItem = -1;

HTTPContext *
NewHTTPContext()
{
	uint64 i = AtomicAddInt32(&HTTPContextArenaLastItem, 1);
	HTTPContext * c = &HTTPContextArena[i];
	Error * err = nil;

	c->RequestBuffer = NewCircularBuffer(2 * 4096, &err);
	if (err != nil) {
		PrintErr("ERROR: failed to create request buffer: ", err);
		return nil;
	}

	c->ResponseBuffer = NewCircularBuffer(2 * (2 * 1024), &err);
	if (err != nil) {
		PrintErr("ERROR: failed to create response buffer: ", err);
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
	Error * err = nil;
	Pool * ctxPool;
	uintptr check;
	Slice date;

	if ((kq = Kqueue()) < 0) {
		Fatal("Failed to open kqueue: ", SyscallErrno);
	}

	EV_SET(&chlist[0], l, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, 0);
	EV_SET(&chlist[1], 1, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, 1, 0);
	if (Kevent(kq, chlist, ArrayLength(chlist), nil, 0, nil) < 0) {
		Fatal("Failed to add event for listener socket: ", SyscallErrno);
	}

	if (ClockGettime(CLOCK_REALTIME, &tp) < 0) {
		Fatal("Failed to get current walltime: ", SyscallErrno);
	}
	tp.tv_nsec = 0;
	date = SliceFrom(dateBuf, sizeof(dateBuf));

	ctxPool = NewPool((NewPoolItemFunc)NewHTTPContext, &err);
	if (err != nil) {
		FatalErr("Failed to create HTTP context pool: ", err);
	}

	while (1) {
		if ((nevents = Kevent(kq, nil, 0, events, ArrayLength(events), nil)) < 0) {
			if (-nevents == EINTR) {
				continue;
			}
			PrintMsgCode("ERROR: failed to get requested kernel events: ", -nevents);
		}

		for (i = 0; i < nevents; ++i) {
			e = &events[i];
			c = e->ident;

			/* PrintKevent(e); */

			if (c == l) {
				void * udata;

				if ((c = Accept(l, nil, 0)) < 0) {
					PrintMsgCode("ERROR: failed to accept connection: ", SyscallErrno);
					continue;
				}

				ctx = PoolGet(ctxPool);
				if (ctx == nil) {
					PrintCStringLn("Failed to acquire new context");
					goto closeConnection;
				}

				udata = (void * )((uintptr)ctx | ctx->Check);
				EV_SET(&chlist[0], c, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, udata);
				EV_SET(&chlist[1], c, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, udata);
				if (Kevent(kq, chlist, ArrayLength(chlist), nil, 0, nil) < 0) {
					PrintMsgCode("ERROR: failed to add new events to kqueue: ", SyscallErrno);
					PoolPut(ctxPool, ctx);
					goto closeConnection;
				}
				continue;
			} else if (c == 1) {
				tp.tv_sec += e->data;
				SlicePutTmRFC822(date, TimeToTm(tp.tv_sec));
				PrintStringLn(StringFrom(date.Base, date.Len));
				continue;
			}

			check = (uintptr)e->udata & 0x1;
			ctx = (void * )((uintptr)ctx - check);
			if (check != ctx->Check) {
				continue;
			}

			switch (e->filter) {
			case EVFILT_READ:
				break;
			case EVFILT_WRITE:
				Write(c, "Hello, world!\n", sizeof("Hello, world!\n") - 1);
				goto closeConnection;
				break;
			}

closeConnection:
			if (ctx != nil) {
				ctx->Check = 1 - ctx->Check;
				PoolPut(ctxPool, ctx);
			}
			Shutdown(c, SHUT_WR);
			Close(c);
		}
	}

	(void)router;
}


Error *
ListenAndServe(int16 port, HTTPRouter router)
{
	struct sockaddr_in addr;
	int	l;

	if ((l = Socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		return NewError("failed to create socket: ", SyscallErrno);
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = SwapBytesInPort(port);
	if (Bind(l, (struct sockaddr *) & addr, sizeof(addr)) < 0) {
		return NewError("failed to bind socket: ", SyscallErrno);
	}

	if (Listen(l, 128) < 0) {
		return NewError("failed to listen on socket: ", SyscallErrno);
	}

	/* TODO(anton2920): add more threads. */
	HTTPWorker(l, router);

	return nil;
}


