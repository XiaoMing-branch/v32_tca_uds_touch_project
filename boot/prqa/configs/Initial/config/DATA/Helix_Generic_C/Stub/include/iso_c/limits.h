#ifndef _LIMITS_H
#define _LIMITS_H

#define CHAR_BIT         8
#define SCHAR_MIN        (-127)
#define SCHAR_MAX        127
#define UCHAR_MAX        255

#ifdef _CHAR_IS_SIGNED
#define CHAR_MIN         SCHAR_MIN
#define CHAR_MAX         SCHAR_MAX
#else
#define CHAR_MIN         0
#define CHAR_MAX         UCHAR_MAX
#endif

#define MB_LEN_MAX       1
#define SHRT_MIN         (-32767)
#define SHRT_MAX         32767
#define USHRT_MAX        65535
#define INT_MIN          LONG_MIN
#define INT_MAX          LONG_MAX
#define UINT_MAX         ULONG_MAX
#define LONG_MIN         (-2147483647)
#define LONG_MAX         2147483647
#define ULONG_MAX        4294967295
#define LLONG_MIN        (-9223372036854775807)
#define LLONG_MAX        9223372036854775807
#define ULLONG_MAX       18446744073709551615

#endif
