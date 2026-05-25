/* stdarg.h	lastmod 18 May 93  SAC
 *			created 18 Dec 87  SAC
 * version:	@(#)stdarg.h	1.4
 * date:		95/03/02
 *
 * history:
 *	18 May 93  SAC  Fixed bug with _VA_LIST (CR1566)
 */
#ifndef included_STDARG_H
#define included_STDARG_H

#ifndef _VA_LIST
#define _VA_LIST
typedef void *				va_list;
#endif
#define va_start(ap, parmN)	((void)((ap)= &(parmN)))
/*
 * NOTE: va_arg doesn't actually *do* anything, but that doesn't matter for
 * static analysis!
 */
#define va_arg(ap, type)		(*(type*)(ap))
#define va_copy(ap, src)  ((void)((ap)=(src)))
#define va_end(ap)			((void)((ap)=0))

#endif
