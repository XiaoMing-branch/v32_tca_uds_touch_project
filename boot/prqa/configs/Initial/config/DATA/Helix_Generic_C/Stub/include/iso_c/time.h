
#pragma PRQA_MESSAGES_OFF 1057,2216,3602,3625

#ifndef _TIME_H
#define _TIME_H
#ifndef _SIZE_T
#define _SIZE_T
typedef PRQA_SIZE_T size_t;
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define CLOCKS_PER_SEC  100

typedef long clock_t;
typedef long time_t;

struct tm
{
  int tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday, tm_yday, tm_isdst;
};

struct timespec
{
  time_t tv_sec;
  long tv_nsec;
};

clock_t       clock (void);

double        difftime (time_t time1,
                        time_t time0);

time_t        mktime (struct tm * timeptr);

time_t        time (time_t * timer);

char *        asctime (const struct tm * timeptr);

char *        ctime (const time_t * timer);

struct tm *   gmtime (const time_t * timer);

struct tm *   localtime (const time_t * timer);

size_t        strftime (char *            restrict s,
                        size_t                     maxsize,
                        const char *      restrict format,
                        const struct tm * restrict timeptr);

#endif
