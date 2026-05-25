/* setjmp.h	lastmod 19 May 92  SAC
 *			created 18 Dec 87  SAC
 * version:	@(#)setjmp.h	1.4
 * date:		95/03/02
 */
#ifndef _SETJMP_H
#define _SETJMP_H
typedef int jmp_buf[16];
#define setjmp(env)		__setjmp(env)
extern int			__setjmp(jmp_buf);
extern void			longjmp(jmp_buf, int);
#endif
