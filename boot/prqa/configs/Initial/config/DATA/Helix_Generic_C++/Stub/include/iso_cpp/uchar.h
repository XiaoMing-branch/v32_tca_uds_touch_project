#ifndef _UCHAR_H
#define _UCHAR_H

#include <stddef.h>
#include <wchar.h>

extern size_t mbrtoc16(char16_t * pc16,
			const char * s, size_t n,
			mbstate_t * p);

extern size_t c16rtomb (char * s, char16_t c16,
			mbstate_t * ps);


extern size_t mbrtoc32 (char32_t * pc32,
			const char * s, size_t n,
			mbstate_t * p);
extern size_t c32rtomb (char * s, char32_t c32,
			mbstate_t * ps);

#endif
