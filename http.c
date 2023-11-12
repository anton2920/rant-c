#include "u.h"
#include "builtin.h"
#include "runtime.h"

#include "assert.h"
#include "atomic.h"
#include "buffer.h"
#include "error.h"
#include "iovec.h"
#include "http.h"
#include "pool.h"
#include "print.h"
#include "slice.h"
#include "string.h"
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
	slice ResponseIovs;
	uint64 ResponsePos;

	HTTPRequestParser Parser;

	uintptr Check;
} HTTPContext;

char	*HTTPResponseBadRequest            = "HTTP/1.1 400 Bad HTTPRequest\r\nContent-Type: text/html\r\nContent-Length: 175\r\nConnection: close\r\n\r\n<!DOCTYPE html><head><title>400 Bad HTTPRequest</title></head><body><h1>400 Bad HTTPRequest</h1><p>Your browser sent a request that this server could not understand.</p></body></html>";
char	*HTTPResponseMethodNotAllowed      = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\nContent-Length: ...\r\nConnection: close\r\n\r\n";
char	*HTTPResponseRequestEntityTooLarge = "HTTP/1.1 413 HTTPRequest Entity Too Large\r\nContent-Type: text/html\r\nConent-Length: ...\r\nConnection: close\r\n\r\n";


int16
SwapBytesInPort(int16 port)
{
	return (port << 8) | ((port & 0xFF00) >> 8);
}


void
WriteResponseNoCopy(HTTPResponse *w, string contentType, slice body)
{
	Iovec * msgs = (Iovec * )w->Iovs->base + w->Iovs->len;
	int	n = SlicePutInt(w->ContentLengthBuf, body.len);
	int	num = 8;

	if (w->Iovs->len + num > w->Iovs->cap) {
		*w->Iovs = growslice(w->Iovs->base, w->Iovs->len + num, w->Iovs->cap, num, sizeof(Iovec));
	}

	msgs[0] = IovecForCString("HTTP/1.1 200 OK\r\nHost: rant\r\nDate: ");
	msgs[1] = IovecForByteSlice(w->DateBuf);
	msgs[2] = IovecForCString("\r\nContent-Type: ");
	msgs[3] = IovecForString(contentType);
	msgs[4] = IovecForCString("\r\nContent-Length: ");
	msgs[5] = IovecForByteSlice(SliceRight(w->ContentLengthBuf, n));
	msgs[6] = IovecForCString("\r\n\r\n");
	msgs[7] = IovecForByteSlice(body);
	w->Iovs->len += num;
}


HTTPContext *
NewHTTPContext(void)
{
	HTTPContext * c;
	error err;

	c = new(HTTPContext);
	c->Parser.State = HTTP_STATE_METHOD;

	c->RequestBuffer = NewCircularBuffer(2 * 4096, &err);
	if (err != nil) {
		PrintError("ERROR: failed to create request buffer:", err);
		return nil;
	}
	c->ResponseIovs = make(Iovec, 0, 256);

	return c;
}


void
HTTPHandleRequests(slice *wIovs, CircularBuffer *rBuf, HTTPRequestParser *rp, slice contentLengthBuf, slice dateBuf, HTTPRouter router)
{
	string unconsumed, httpVersionPrefix, httpVersion, header;
	int64 lineEnd, uriEnd, queryStart;
	HTTPRequest * r = &rp->Request;
	HTTPResponse w;
	(void)w;

	while (1) {
		while (rp->State != HTTP_STATE_DONE) {
			switch (rp->State) {
			case HTTP_STATE_UNKNOWN:
				unconsumed = UnconsumedString(rBuf);
				if (unconsumed.len < 2) {
					return;
				}
				if ((unconsumed.base[0] == '\r') && (unconsumed.base[1] == '\n')) {
					Consume(rBuf, sizeof("\r\n") - 1);
					rp->State = HTTP_STATE_DONE;
				} else {
					rp->State = HTTP_STATE_HEADER;
				}
				break;

			case HTTP_STATE_METHOD:
				unconsumed = UnconsumedString(rBuf);
				if (unconsumed.len < 3) {
					return;
				}
				if ((unconsumed.base[0] == 'G') && (unconsumed.base[1] == 'E') && (unconsumed.base[2] == 'T')) {
					r->Method = StringLiteral("GET");
				} else {
					Iovec msg = IovecForCString(HTTPResponseMethodNotAllowed);
					append(*wIovs, msg);
					return;
				}
				Consume(rBuf, r->Method.len + 1);
				rp->State = HTTP_STATE_URI;
				break;

			case HTTP_STATE_URI:
				unconsumed = UnconsumedString(rBuf);
				lineEnd = FindChar(unconsumed, '\r');
				if (lineEnd == -1) {
					return;
				}

				uriEnd = FindChar(StringRight(unconsumed, lineEnd), ' ');
				if (uriEnd == -1) {
					Iovec msg = IovecForCString(HTTPResponseBadRequest);
					append(*wIovs, msg);
					return;
				}

				queryStart = FindChar(StringRight(unconsumed, lineEnd), '?');
				if (queryStart != -1) {
					r->URL.Path = StringRight(unconsumed, queryStart);
					r->URL.Query = StringLeftRight(unconsumed, queryStart + 1, uriEnd);
				} else {
					r->URL.Path = StringRight(unconsumed, uriEnd);
					r->URL.Query = StringLiteral("");
				}

				httpVersionPrefix = StringLiteral("HTTP/");
				httpVersion = StringLeftRight(unconsumed, uriEnd + 1, lineEnd);
				if (!memequal(httpVersion.base, httpVersionPrefix.base, httpVersionPrefix.len)) {
					Iovec msg = IovecForCString(HTTPResponseBadRequest);
					append(*wIovs, msg);
					return;
				}
				r->Version = StringLeft(httpVersion, httpVersionPrefix.len);
				Consume(rBuf, r->URL.Path.len + r->URL.Query.len + 1 + httpVersionPrefix.len + r->Version.len + sizeof("\r\n") - 1);
				rp->State = HTTP_STATE_UNKNOWN;
				break;

			case HTTP_STATE_HEADER:
				unconsumed = UnconsumedString(rBuf);
				lineEnd = FindChar(unconsumed, '\r');
				if (lineEnd == -1) {
					return;
				}

				header = StringRight(unconsumed, lineEnd);
				Consume(rBuf, header.len + sizeof("\r\n") - 1);
				rp->State = HTTP_STATE_UNKNOWN;
				break;

			default:
				PrintInt(rp->State);
				panic("unknown HTTP parser state");
				break;
			}
		}

		w.Iovs = wIovs;
		w.DateBuf = dateBuf;
		w.ContentLength = 0;
		w.ContentLengthBuf = contentLengthBuf;

		router(&w, r);

		/* PrintRequest(r); */

		rp->State = HTTP_STATE_METHOD;
	}
}


void
HTTPWorker(int l, HTTPRouter router)
{
	char	contentLengthArray[10], dateArray[31];
	struct kevent chlist[2], events[256], *e;
	slice contentLengthBuf, dateBuf;
	HTTPRequestParser * parser;
	int	kq, nevents, i, c;
	CircularBuffer * rBuf;
	HTTPContext * ctx;
	struct timespec tp;
	Pool * ctxPool;
	uintptr check;
	slice wIovs;
	error err;
	int64 n;

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
	dateBuf = UnsafeSlice(dateArray, sizeof(dateArray));

	contentLengthBuf = UnsafeSlice(contentLengthArray, sizeof(contentLengthArray));

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
					PrintCString("ERROR: failed to acquire new context");
					goto closeConnection;
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
				SlicePutTmRFC822(dateBuf, TimeToTm(tp.tv_sec));
				continue;
			}

			check = (uintptr)e->udata & 0x1;
			ctx = (void * )((uintptr)e->udata - check);
			if (check != ctx->Check) {
				continue;
			}

			switch (e->filter) {
			case EVFILT_READ:
				if (e->flags & EV_EOF) {
					goto closeConnection;
				}

				rBuf = &ctx->RequestBuffer;
				parser = &ctx->Parser;

				/* Breakpoint; */

				if (RemainingSpace(rBuf) == 0) {
					struct iovec msg = IovecForCString(HTTPResponseRequestEntityTooLarge);
					Shutdown(c, SHUT_RD);
					Writev(c, UnsafeSlice(&msg, 1), nil);
					goto closeConnection;
				}

				n = Read(c, RemainingSlice(rBuf), &err);
				if (err != nil) {
					PrintError("ERROR: failed to read data from socket:", err);
					goto closeConnection;
				}
				Produce(rBuf, n);

				HTTPHandleRequests(&ctx->ResponseIovs, rBuf, parser, contentLengthBuf, dateBuf, router);

				wIovs = ctx->ResponseIovs;
				Writev(c, SliceLeft(wIovs, ctx->ResponsePos), &err);
				if (err != nil) {
					PrintError("ERROR: failed to write data to socket:", err);
					goto closeConnection;
				}

				/* TODO(anton2920): for now assume all is written. */
				ctx->ResponseIovs.len = 0;

				break;
			case EVFILT_WRITE:
				if (e->flags & EV_EOF) {
					PrintCString("I'm here!!!");
					goto closeConnection;
				}

				wIovs = ctx->ResponseIovs;
				if (SliceLeft(wIovs, ctx->ResponsePos).len > 0) {
					wIovs = ctx->ResponseIovs;
					Writev(c, SliceLeft(wIovs, ctx->ResponsePos), &err);
					if (err != nil) {
						PrintError("ERROR: failed to write data to socket:", err);
						goto closeConnection;
					}

					/* TODO(anton2920): for now assume all is written. */
					ctx->ResponseIovs.len = 0;
				}
				break;
			}
			continue;

closeConnection:
			if (ctx != nil) {
				ctx->Check = 1 - ctx->Check;
				Reset(&ctx->RequestBuffer);
				PoolPut(ctxPool, ctx);
			}

			Shutdown(c, SHUT_WR);
			Close(c);
		}
	}
}


error
ListenAndServe(int16 port, HTTPRouter router)
{
	struct sockaddr_in addr;
	int	nworkers = 4;
	int	enable = 1;
	int	l, i, pid;
	error err;

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

	for (i = 0; i < nworkers - 1; ++i) {
		pid = Fork(nil);
		if (pid == 0) {
			break;
		}
	}
	HTTPWorker(l, router);

	return nil;
}


