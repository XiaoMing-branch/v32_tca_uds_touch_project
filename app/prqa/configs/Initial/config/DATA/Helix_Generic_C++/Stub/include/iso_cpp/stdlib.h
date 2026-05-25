/* stdlib.h  lastmod 19 Nov 91  SAC
 *      created 18 Dec 87  SAC
 * version:  @(#)stdlib.h  1.3
 * date:    95/03/02
 */

#ifndef _STDLIB_H
#define _STDLIB_H

#include "qac_size_t.h"
#include "qac_wchar_t.h"

#ifndef NULL
#define NULL        (0)
#endif

extern "C++"
{
  extern int    atexit(void (*)(void)), at_quick_exit(void (*)(void));
}

extern "C"
{
  struct div_t {
    int quot;
    int rem;
  };

  struct ldiv_t {
    long int quot;
    long int rem;
  };

#define EXIT_FAILURE    1
#define EXIT_SUCCESS    0
#define RAND_MAX      32767
#define MB_CUR_MAX      1

  extern double  atof(const char *), strtod(const char *, char **);
  extern float  strtof(const char *, char **);
  extern long double  strtold(const char *, char **);
  extern long  atol(const char *), strtol(const char *, char **, int), labs(long);
  extern long long int atoll (const char *), strtoll (const char *, char **, int),
    llabs(long long int);
  extern unsigned long  strtoul(const char *, char **, int);
  extern unsigned long long  strtoull(const char *, char **, int);
  extern int  atoi(const char *), rand(void),
      system(const char *), abs(int), mblen(const char *, size_t),
      mbtowc(wchar_t *, const char *, size_t), wctomb(char *, wchar_t);
  extern void  srand(unsigned int), *aligned_alloc(size_t, size_t), *calloc(size_t, size_t), free(void *),
      *malloc(size_t), *realloc(void *, size_t), abort(void), exit(int), _Exit(int), quick_exit(int),
      *bsearch(const void*,const void*,size_t,size_t,int(*)(const void*,const void*)),
      qsort(void*,size_t,size_t,int(*)(const void*,const void*));
  void *aligned_alloc (size_t, size_t);  /* introduced in C11 */
  extern char  *getenv(const char *);
  extern int putenv(char *);
  extern div_t  div(int, int);
  extern ldiv_t  ldiv(long, long);
  extern size_t  mbstowcs(wchar_t*,const char*,size_t),
      wcstombs(char*,const wchar_t*,size_t);
}

#endif
