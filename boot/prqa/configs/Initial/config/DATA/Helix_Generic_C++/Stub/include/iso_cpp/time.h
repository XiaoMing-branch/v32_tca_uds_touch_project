/* time.h		lastmod 19 Nov 91  SAC
 *			created 18 Dec 87  SAC
 * version:	@(#)time.h	1.3
 * date:		95/03/02
 */
#ifndef _TIME_H
#define _TIME_H

#include "qac_size_t.h"

#ifndef NULL
#define NULL		(0)
#endif

#define CLOCKS_PER_SEC	100
typedef long clock_t;
typedef long time_t;
struct tm
{ int tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday, tm_yday, tm_isdst; };
struct timespec
{ time_t tv_sec; long tv_nsec; };

extern clock_t	clock(void);
extern double	difftime(time_t, time_t);
extern time_t	mktime(struct tm *), time(time_t *);
extern char	*asctime(const struct tm *), *ctime(const time_t *);
extern struct tm *gmtime(const time_t *), *localtime(const time_t *);
extern size_t	strftime(char *, size_t, const char *, const struct tm *);

extern int nanosleep(const struct timespec *req, struct timespec *rem);

#endif
