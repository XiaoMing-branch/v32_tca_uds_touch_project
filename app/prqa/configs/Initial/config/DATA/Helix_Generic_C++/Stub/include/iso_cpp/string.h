/* string.h	lastmod 19 Nov 91  SAC
 *			created 18 Dec 87  SAC
 * version:	@(#)string.h	1.3
 * date:		95/03/02
 */
#ifndef _STRING_H
#define _STRING_H

#include "qac_size_t.h"

#ifndef NULL
#define NULL		(0)
#endif
extern void	*memcpy(void*,const void*,size_t),
		*memmove(void*,const void*,size_t),
		*memchr(const void *, int, size_t),
		*memset(void *, int, size_t);
extern char	*strcpy(char *, const char *),
		*strncpy(char *, const char *, size_t),
		*strcat(char *, const char *),
		*strncat(char *, const char *, size_t),
		*strchr(const char *, int),
		*strpbrk(const char *, const char *),
		*strrchr(const char *, int),
		*strstr(const char *, const char *),
		*strtok(char *, const char *),
		*strerror(int);
extern int	memcmp(const void*,const void*,size_t),
		strcmp(const char*,const char*),
		strcoll(const char*,const char*),
		strncmp(const char*,const char*,size_t);
extern size_t	strxfrm(char*,const char*,size_t),
		strcspn(const char*,const char*),
		strspn(const char *, const char *),
		strlen(const char *);
extern char	*strdup(const char*);
extern char *strndup(const char *, size_t);
#endif
