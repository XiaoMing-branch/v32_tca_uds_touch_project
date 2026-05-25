#ifndef included_WCHAR_H
#define included_WCHAR_H

#include "qac_size_t.h"
#include "qac_wchar_t.h"

#include <stdio.h>

//empty

typedef int mbstate_t;
typedef int wint_t;

//#define NULL (0)
//<either 0, 0L, or (void *)0> [0 in C++]
#define WCHAR_MAX (127)
//<#if expression >= 127>
#define WCHAR_MIN (0)
//<#if expression <= 0>
#define WEOF (wint_t(0))
//<wint_t constant expression>
wint_t btowc(int c);
wint_t fgetwc(FILE *stream);
wchar_t *fgetws(wchar_t *s, int n, FILE *stream);
wint_t fputwc(wchar_t c, FILE *stream);
int fputws(const wchar_t *s, FILE *stream);
int fwide(FILE *stream, int mode);
int fwprintf(FILE *stream, const wchar_t *format, ...);
int fwscanf(FILE *stream, const wchar_t *format, ...);
wint_t getwc(FILE *stream);
wint_t getwchar(void);
size_t mbrlen(const char *s, size_t n, mbstate_t *ps);
size_t mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps);
int mbsinit(const mbstate_t *ps);
size_t mbsrtowcs(wchar_t *dst, const char **src, size_t len, mbstate_t *ps);
//typedef o-type mbstate_t;
wint_t putwc(wchar_t c, FILE *stream);
wint_t putwchar(wchar_t c);
int swprintf(wchar_t *s, size_t n, const wchar_t *format, ...);
int swscanf(const wchar_t *s, const wchar_t *format, ...);
struct tm;
wint_t ungetwc(wint_t c, FILE *stream);
int vfwprintf(FILE *stream, const wchar_t *format, va_list arg);
int vfwscanf(FILE *stream, const wchar_t *format, va_list arg);
int vswprintf(wchar_t *s, size_t n, const wchar_t *format, va_list arg);
int vwprintf(const wchar_t *format, va_list arg);
size_t wcrtomb(char *s, wchar_t wc, mbstate_t *ps);
wchar_t *wcscat(wchar_t *s1, const wchar_t *s2);
wchar_t *wcschr(const wchar_t *s, wchar_t c);
int wcscmp(const wchar_t *s1, const wchar_t *s2);
int wcscoll(const wchar_t *s1, const wchar_t *s2);
wchar_t *wcscpy(wchar_t *s1, const wchar_t *s2);
size_t wcscspn(const wchar_t *s1, const wchar_t *s2);
size_t wcsftime(wchar_t *s, size_t maxsize, const wchar_t *format, const struct tm *timeptr);
size_t wcslen(const wchar_t *s);
wchar_t *wcsncat(wchar_t *s1, const wchar_t *s2, size_t n);
int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n);
wchar_t *wcsncpy(wchar_t *s1, const wchar_t *s2, size_t n);
wchar_t *wcspbrk(const wchar_t *s1, const wchar_t *s2);
wchar_t *wcsrchr(const wchar_t *s, wchar_t c);
size_t wcsrtombs(char *dst, const wchar_t **src, size_t len, mbstate_t *ps);
size_t wcsspn(const wchar_t *s1, const wchar_t *s2);
wchar_t *wcsstr(const wchar_t *s1, const wchar_t *s2);
float wcstof(const wchar_t *nptr, wchar_t **endptr);
double wcstod(const wchar_t *nptr, wchar_t **endptr);
long double wcstold(const wchar_t *nptr, wchar_t **endptr);
wchar_t *wcstok(wchar_t *s1, const wchar_t *s2, wchar_t **ptr);
long wcstol(const wchar_t *nptr, wchar_t **endptr, int base);
long long wcstoll(const wchar_t *nptr, wchar_t **endptr, int base);
unsigned long wcstoul(const wchar_t *nptr, wchar_t **endptr, int base);
unsigned long long wcstoull(const wchar_t *nptr, wchar_t **endptr, int base);
size_t wcsxfrm(wchar_t *s1, const wchar_t *s2, size_t n);
int wctob(wint_t c);
//typedef i_type wint_t;
wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n);
int wmemcmp(const wchar_t *s1, const wchar_t *s2, size_t n);
wchar_t *wmemcpy(wchar_t *s1, const wchar_t *s2, size_t n);
wchar_t *wmemmove(wchar_t *s1, const wchar_t *s2, size_t n);
wchar_t *wmemset(wchar_t *s, wchar_t c, size_t n);
int wprintf(const wchar_t *format, ...);
int wscanf(const wchar_t *format, ...); 

#endif
