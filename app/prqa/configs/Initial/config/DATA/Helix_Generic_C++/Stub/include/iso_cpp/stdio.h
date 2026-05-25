/* stdio.h	lastmod 18 May 93  SAC
 *			created 18 Dec 87  SAC
 * version:	@(#)stdio.h	1.5
 * date:		95/03/02
 *
 * history:
 *	18 May 93  SAC  Fixed bug with _VA_LIST (CR1566)
 */
#ifndef included_STDIO_H
#define included_STDIO_H

#include "qac_size_t.h"

#ifndef NULL
#define NULL			(0)
#endif
#define _IOFBF			0000
#define _IONBF			0004
#define _IOLBF			0200
#define BUFSIZ			1024
#define EOF			(-1)
#define FOPEN_MAX		8
#define FILENAME_MAX	76
#define L_tmpnam		12
#define SEEK_SET		0
#define SEEK_CUR		1
#define SEEK_END		2
#define TMP_MAX		25
typedef unsigned long	fpos_t;
#ifdef	sun
/*
 * For some reason, Sun use a define instead of a typedef
 */
struct _iobuf { unsigned __the_contents_of_FILE_are_not_defined; };
#define FILE			struct _iobuf
#else
typedef struct FILE { unsigned __the_contents_of_FILE_are_not_defined; } FILE;
#endif
extern FILE			*stderr, *stdin, *stdout;
#ifndef _VA_LIST
#define _VA_LIST
typedef void *			va_list;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  
extern int	remove(const char *), rename(const char *, const char *),
			fclose(FILE *), fflush(FILE *), setvbuf(FILE *, char *, int, size_t),
			fprintf(FILE *, const char *, ...), fscanf(FILE *, const char *, ...),
			printf(const char *, ...), scanf(const char *, ...),
			sprintf(char *, const char *, ...),snprintf(char *, size_t, const char *, ...),
			sscanf(const char *, const char *, ...),
			vfprintf(FILE *, const char *, va_list), vprintf(const char *, va_list),
			vsnprintf(char *s, size_t n, const char *format, va_list ap),
			vsprintf(char *, const char *, va_list), fgetc(FILE *),
			fputc(int, FILE *), fputs(const char *, FILE *), getc(FILE *),
			getchar(void), putc(int, FILE *), putchar(int), puts(const char *),
			ungetc(int, FILE *), fgetpos(FILE *, fpos_t *),
			fseek(FILE *, long int, int), fsetpos(FILE *, const fpos_t *),
			feof(FILE *), ferror(FILE *);
extern FILE	*tmpfile(void), *fopen(const char *, const char *),
			*freopen(const char *, const char *, FILE *);
extern char	*tmpnam(char *), *fgets(char *, int, FILE *), *gets(char *);
extern size_t	fread(void *, size_t, size_t, FILE *),
			fwrite(const void *, size_t, size_t, FILE *);
extern long	ftell(FILE *);
extern void	setbuf(FILE *, char *), rewind(FILE *), clearerr(FILE *),
			perror(const char *);

#ifdef __cplusplus
}
#endif

#endif
