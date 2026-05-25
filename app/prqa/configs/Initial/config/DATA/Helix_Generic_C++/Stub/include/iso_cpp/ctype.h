/* ctype.h	lastmod 19 Nov 91  SAC
 *			created 18 Dec 87  SAC
 * version:	@(#)ctype.h	1.3
 * date:		95/03/02
 */
#ifndef _CTYPE_H
#define _CTYPE_H
int	isalnum(int), isalpha(int), iscntrl(int), isdigit(int), isgraph(int), islower(int),
	isprint(int), ispunct(int), isspace(int), isupper(int), isxdigit(int),
	tolower(int), toupper(int);

	// to ease testing in qacpp the ISOC99 def is defaulted for c++11 and later
	#if __cplusplus >= 201103L
	# define __USE_ISOC99
	#endif

	#ifdef __USE_ISOC99
	int isblank(int);
	#endif
#endif
