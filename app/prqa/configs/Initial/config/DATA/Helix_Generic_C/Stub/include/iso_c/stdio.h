
#pragma PRQA_MESSAGES_OFF 336,776,815
#pragma PRQA_MESSAGES_OFF 1011,1057,1300,1307,2051
#pragma PRQA_MESSAGES_OFF 3602,3625

#ifndef included_STDIO_H
#define included_STDIO_H

#ifndef NULL
#define NULL            ((void *) 0)
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef PRQA_SIZE_T size_t;
#endif

#define _IOFBF          0000
#define _IONBF          0004
#define _IOLBF          0200
#define BUFSIZ          1024
#define EOF             (-1)
#define FOPEN_MAX       8
#define FILENAME_MAX    76
#define L_tmpnam        12
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2
#define TMP_MAX         25
typedef unsigned long   fpos_t;

// For some reason, Sun use a define instead of a typedef to define FILE
#ifndef _FILE_DEFINED
#define _FILE_DEFINED
#ifdef  sun
struct _iobuf { unsigned __the_contents_of_FILE_are_not_defined; };
#define FILE  struct _iobuf
#else
typedef struct { unsigned __the_contents_of_FILE_are_not_defined; } FILE;
#endif
#endif

extern FILE *stderr, *stdin, *stdout;

#ifndef _VA_LIST
#define _VA_LIST
typedef void * va_list;
#endif

int       remove (const char * filename);

int       rename (const char * old,
                  const char * new);

FILE *    tmpfile (void);

char *    tmpnam (char * s);

int       fclose (FILE * stream);

int       fflush (FILE * stream);

FILE *    fopen (const char * restrict filename,
                 const char * restrict mode);

FILE *    freopen (const char * restrict filename,
                   const char * restrict mode,
                   FILE *       restrict stream);

void      setbuf (FILE * restrict stream,
                  char * restrict buf);

int       setvbuf (FILE * restrict stream,
                   char * restrict buf,
                   int             mode,
                   size_t          size);

int       fprintf (FILE *       restrict stream,
                   const char * restrict format,
                   ...);

int       fscanf (FILE *       restrict stream,
                  const char * restrict format,
                  ...);

int       printf (const char * restrict format,
                  ...);

int       scanf (const char * restrict format,
                 ...);

int       snprintf (char *       restrict s,
                    size_t                n,
                    const char * restrict format,
                    ...);

int       sprintf (char *       restrict s,
                   const char * restrict format,
                   ...);

int       sscanf (const char * restrict s,
                  const char * restrict format,
                  ...);

int       vfprintf (FILE *       restrict stream,
                    const char * restrict format,
                    va_list               arg);

int       vfscanf (FILE *       restrict stream,
                   const char * restrict format,
                   va_list               arg);

int       vprintf (const char * restrict format,
                   va_list               arg);

int       vscanf (const char * restrict format,
                  va_list               arg);

int       vsnprintf (char *       restrict s,
                     size_t                n,
                     const char * restrict format,
                     va_list               arg);

int       vsprintf (char *       restrict s,
                    const char * restrict format,
                    va_list               arg);

int       vsscanf (const char * restrict str,
                   const char * restrict format,
                   va_list               arg);

int       fgetc (FILE * stream);

char *    fgets (char * restrict s,
                 int             n,
                 FILE * restrict stream);

int       fputc (int    c,
                 FILE * stream);

int       fputs (const char * restrict s,
                 FILE       * restrict stream);

int       getc (FILE * stream);

int       getchar (void);

char *    gets (char * s);

int       putc (int    c,
                FILE * stream);

int       putchar (int c);

int       puts (const char *s);

int       ungetc (int    c,
                  FILE * stream);

size_t    fread (void * restrict ptr,
                 size_t          size,
                 size_t          nmemb,
                 FILE * restrict stream);

size_t    fwrite (const void * restrict ptr,
                  size_t                size,
                  size_t                nmemb,
                  FILE *       restrict stream);

int       fgetpos (FILE *   restrict stream,
                   fpos_t * restrict pos);

int       fseek (FILE *   stream,
                 long int offset,
                 int      whence);

int       fsetpos (FILE *         stream,
                   const fpos_t * pos);

long int  ftell (FILE * stream);

void      rewind (FILE * stream);

void      clearerr (FILE * stream);

int       feof (FILE * stream);

int       ferror (FILE * stream);

void      perror (const char * s);

// Annex K (incomplete)

#include <_annexK.h>
#if __PRQA_HAVE_ANNEX_K

#ifndef _RSIZE_T
#define _RSIZE_T
typedef size_t rsize_t;
#endif

int printf_s(const char *restrict format, ...);
int fprintf_s(FILE *restrict stream, const char *restrict format, ...);
int sprintf_s(char *restrict buffer, rsize_t bufsz, const char *restrict format, ...);
int snprintf_s(char *restrict buffer, rsize_t bufsz, const char *restrict format, ...);
int scanf_s(const char *restrict format, ...);
int fscanf_s(FILE *restrict stream, const char *restrict format, ...);
int sscanf_s(const char *restrict buffer, const char *restrict format, ...);

#endif

#endif
