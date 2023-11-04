#include <time.h>

#include "u.h"

struct tm TimeToTm(int64 t)
{
	int64 quadricentennials, centennials, quadrennials, annuals;
	int64 yday, mday, wday;
	int64 year, month, leap;
	int64 hour, min, sec;
	struct tm tm;

	int	daysSinceJan1st[][13] = {
		{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 }, /* non-leap year. */
		{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }, /* leap year. */
	};

	t += 3 * 60 * 60; /* MSK timezone hack. */

	/* Re-bias from 1970 to 1601: 1970 - 1601 = 369 = 3*100 + 17*4 + 1 years (incl. 89 leap days) = (3*100*(365+24/100) + 17*4*(365+1/4) + 1*365)*24*3600 seconds. */
	sec = t + 11644473600;

	wday = (sec / 86400 + 1) % 7; /* day of week */

	/* Remove multiples of 400 years (incl. 97 leap days). */
	quadricentennials = sec / 12622780800; /* 400*365.2425*24*3600 .*/
	sec %= 12622780800;

	/* Remove multiples of 100 years (incl. 24 leap days), can't be more than 3 (because multiples of 4*100=400 years (incl. leap days) have been removed). */
	centennials = sec / 3155673600; /* 100*(365+24/100)*24*3600. */
	if (centennials > 3) {
		centennials = 3;
	}
	sec -= centennials * 3155673600;

	/* Remove multiples of 4 years (incl. 1 leap day), can't be more than 24 (because multiples of 25*4=100 years (incl. leap days) have been removed). */
	quadrennials = sec / 126230400; /*  4*(365+1/4)*24*3600. */
	if (quadrennials > 24) {
		quadrennials = 24;
	}
	sec -= quadrennials * 126230400;

	/* Remove multiples of years (incl. 0 leap days), can't be more than 3 (because multiples of 4 years (incl. leap days) have been removed). */
	annuals = sec / 31536000; /* 365*24*3600 */
	if (annuals > 3) {
		annuals = 3;
	}
	sec -= annuals * 31536000;

	/* Calculate the year and find out if it's leap. */
	year = 1601 + quadricentennials * 400 + centennials * 100 + quadrennials * 4 + annuals;
	leap = (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));

	/* Calculate the day of the year and the time. */
	yday = sec / 86400;
	sec %= 86400;
	hour = sec / 3600;
	sec %= 3600;
	min  = sec / 60;
	sec %= 60;

	/* Calculate the month. */
	for (month = 1, mday = 1; month <= 12; ++month) {
		if (yday < daysSinceJan1st[leap][month]) {
			mday += yday - daysSinceJan1st[leap][month-1];
			break;
		}
	}

	tm.tm_sec = sec;          /*  [0,59]. */
	tm.tm_min = min;          /*  [0,59]. */
	tm.tm_hour = hour;        /*  [0,23]. */
	tm.tm_mday = mday;        /*  [1,31]  (day of month). */
	tm.tm_mon = month - 1;    /*  [0,11]  (month). */
	tm.tm_year = year - 1900; /*  70+     (year since 1900). */
	tm.tm_wday = wday;        /*  [0,6]   (day since Sunday AKA day of week). */
	tm.tm_yday = yday;        /*  [0,365] (day since January 1st AKA day of year). */
	tm.tm_isdst = -1;         /*  daylight saving time flag. */

	return tm;
}


