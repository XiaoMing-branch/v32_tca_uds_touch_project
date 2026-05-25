
#ifndef included_WTYPE_H
#define included_WTYPE_H

#include <wchar.h>

typedef int wctype_t;
typedef int wctrans_t;


int iswalnum(wint_t c);
int iswalpha(wint_t c);
int iswcntrl(wint_t c);
int iswctype(wint_t c, wctype_t category);
int iswdigit(wint_t c);
int iswgraph(wint_t c);
int iswlower(wint_t c);
int iswprint(wint_t c);
int iswpunct(wint_t c);
int iswspace(wint_t c);
int iswupper(wint_t c);
int iswxdigit(wint_t c);
wint_t towctrans(wint_t c, wctrans_t category);
wint_t towlower(wint_t c);
wint_t towupper(wint_t c);
wctrans_t wctrans(const char *property);
//typedef s_type wctrans_t;
wctype_t wctype(const char *property);
//typedef i_type wint_t;

// to ease testing in qacpp the ISOC99 def is defaulted for c++11 and later
#  if __cplusplus >= 201103L
#   define __USE_ISOC99
#  endif

#  ifdef __USE_ISOC99
int iswblank(wint_t wc);
#  endif

#endif
