#pragma PRQA_MESSAGES_OFF 631,815

#ifndef _ERRNO_H
#define _ERRNO_H

#define EDOM    1
#define ERANGE  2
#define ECHILD  10
#define EINTR   3407
extern int errno;

#include <_annexK.h>
#if __PRQA_HAVE_ANNEX_K

#ifndef _ERRNO_T
#define _ERRNO_T
typedef int errno_t;
#endif

#endif

#endif
