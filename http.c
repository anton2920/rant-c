#include <netinet/in.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>

#include "u.h"
#include "slice.h"
#include "string.h"
#include "url.h"

#include "error.h"
#include "http.h"
#include "print.h"
#include "syscall.h"
#include "time.h"
#include "utils.h"

int16
SwapBytesInPort(int16 port)
{
	return (port << 8) | ((port & 0xFF00) >> 8);
}


void
HTTPWorker(int l, HTTPRouter router)
{
	struct kevent chlist[2], events[256], *e;
	int	kq, ret, nevents, i, c;
	struct timespec tp;
	char	dateBuf[31];
	Slice date;

	if ((kq = Kqueue()) < 0) {
		Fatal("Failed to open kqueue: ", kq);
	}

	EV_SET(&chlist[0], l, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, 0);
	EV_SET(&chlist[1], 1, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, 1, 0);
	if ((ret = Kevent(kq, chlist, ArrayLength(chlist), nil, 0, nil)) < 0) {
		Fatal("Failed to add event for listener socket: ", ret);
	}

	if ((ret = ClockGettime(CLOCK_REALTIME, &tp)) < 0) {
		Fatal("Failed to get current walltime: ", ret);
	}
	tp.tv_nsec = 0;
	date = SliceFrom(dateBuf, sizeof(dateBuf));

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
				if ((c = Accept(l, nil, 0)) < 0) {
					PrintMsgCode("ERROR: failed to accept connection: ", -c);
					continue;
				}

				Write(c, "Test message from RANT (C edition)\n", sizeof("Test message from RANT (C edition)\n") - 1);
				Close(c);
			} else if (c == 1) {
				tp.tv_sec += e->data;
				SlicePutTmRFC822(date, TimeToTm(tp.tv_sec));
				PrintStringLn(StringFrom(date.Base, date.Len));
				continue;
			}
		}
	}

	(void)date;
	(void)router;
}


Error *
ListenAndServe(int16 port, HTTPRouter router)
{
	struct sockaddr_in addr;
	int	l, ret;

	if ((l = Socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		return NewError("failed to create socket: ", l);
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = SwapBytesInPort(port);
	if ((ret = Bind(l, (struct sockaddr *) & addr, sizeof(addr))) < 0) {
		return NewError("failed to bind socket: ", l);
	}

	if ((ret = Listen(l, 128)) < 0) {
		return NewError("failed to listen on socket: ", ret);
	}

	/* TODO(anton2920): add more threads. */
	HTTPWorker(l, router);

	return nil;
}


