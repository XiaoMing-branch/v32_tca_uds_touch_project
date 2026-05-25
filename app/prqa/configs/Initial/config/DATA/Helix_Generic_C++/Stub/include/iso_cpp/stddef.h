/* stddef.h	lastmod 19 Nov 91  SAC
 *			created 18 Dec 87  SAC
 * version:	@(#)stddef.h	1.3
 * date:		95/03/02
 *
 * LJF changed size_t to unsigned int as used by VC++
 * slight mod by paulfitz Wed Jun  2 15:59:23 IST 1999
 */

#ifndef _STDDEF_H
#define _STDDEF_H

#include "qac_size_t.h"
#include "qac_ptrdiff_t.h"
#include "qac_wchar_t.h"

#ifndef NULL
#define NULL		(0)
#endif

/*
 * offsetof is subtly non-ANSI
 */

#define offsetof(type, member) __qacpp_offsetof(type, member)

#endif
