#pragma PRQA_MESSAGES_OFF 602,3410,3429,3432,3435,3436,3443,3453,3456,3602,4601
#ifndef _TGMATH_H
#define _TGMATH_H

#include <math.h>
#include <complex.h>

/* We don't really support complex numbers, we ignore the imaginary part. */

#define __TGM_LONG(fct) fct ## l

#define __TGM_FUNC(value, Fct) \
              ((sizeof(value) == sizeof(double)) ? Fct(value)	\
                                                 : ((sizeof(value) == sizeof(float)) ? Fct##f(value) \
                                                                                    : __TGM_LONG(Fct) (value)))
#define __TGM_FUNC2(value, value2, Fct) \
              ((sizeof(value) == sizeof(double)) ? Fct(value, value2)	\
                                                 : ((sizeof(value) == sizeof(float)) ? Fct##f(value, value2) \
                                                                                    : __TGM_LONG(Fct) (value, value2)))
#define __TGM_FUNC3(value, value2, value3, Fct) \
              ((sizeof(value) == sizeof(double)) ? Fct(value, value2, value3)	\
                                                 : ((sizeof(value) == sizeof(float)) ? Fct##f(value, value2, value3) \
                                                                                    : __TGM_LONG(Fct) (value, value2, value3)))


#define acos(value) __TGM_FUNC(value, acos)
#define asin(value) __TGM_FUNC(value, asin)
#define atan(value) __TGM_FUNC(value, atan)
#define acosh(value) __TGM_FUNC(value, acosh)
#define asinh(value) __TGM_FUNC(value, asinh)
#define atanh(value) __TGM_FUNC(value, atanh)
#define cos(value) __TGM_FUNC(value, cos)
#define sin(value) __TGM_FUNC(value, sin)
#define tan(value) __TGM_FUNC(value, tan)
#define cosh(value) __TGM_FUNC(value, cosh)
#define sinh(value) __TGM_FUNC(value, sinh)
#define tanh(value) __TGM_FUNC(value, tanh)
#define exp(value) __TGM_FUNC(value, exp)
#define log(value) __TGM_FUNC(value, log)
#define pow(value, value2) __TGM_FUNC2(value, value2, pow)
#define sqrt(value) __TGM_FUNC(value, sqrt)
#define fabs(value) __TGM_FUNC(value, fabs)
#define atan2(value, value2) __TGM_FUNC2(value, value2, atan2)
#define cbrt(value) __TGM_FUNC(value, cbrt)
#define ceil(value) __TGM_FUNC(value, ceil)
#define copysign(value, value2) __TGM_FUNC2(value, value2, copysign)
#define erf(value) __TGM_FUNC(value, erf)
#define erfc(value) __TGM_FUNC(value, erfc)
#define exp2(value) __TGM_FUNC(value, exp2)
#define expm1(value) __TGM_FUNC(value, expm1)
#define fdim(value, value2) __TGM_FUNC2(value, value2, fdim)
#define floor(value) __TGM_FUNC(value, floor)
#define fma(value, value2, value3) __TGM_FUNC3(value, value2, value3, fma)
#define fmax(value, value2) __TGM_FUNC2(value, value2, fmax)
#define fmin(value, value2) __TGM_FUNC2(value, value2, fmin)
#define fmod(value, value2) __TGM_FUNC2(value, value2, fmod)
#define frexp(value, value2) __TGM_FUNC2(value, value2, frexp)
#define hypot(value, value2) __TGM_FUNC2(value, value2, hypot)
#define ilogb(value) __TGM_FUNC(value, ilogb)
#define ldexp(value, value2) __TGM_FUNC2(value, value2, ldexp)
#define lgamma(value) __TGM_FUNC(value, lgamma)
#define llrint(value) __TGM_FUNC(value, llrint)
#define llround(value) __TGM_FUNC(value, llround)
#define log10(value) __TGM_FUNC(value, log10)
#define log1p(value) __TGM_FUNC(value, log1p)
#define log2(value) __TGM_FUNC(value, log2)
#define logb(value) __TGM_FUNC(value, logb)
#define lrint(value) __TGM_FUNC(value, lrint)
#define lround(value) __TGM_FUNC(value, lround)
#define nearbyint(value) __TGM_FUNC(value, nearbyint)
#define nextafter(value, value2) __TGM_FUNC2(value, value2, nextafter)
#define nexttoward(value, value2) __TGM_FUNC2(value, value2, nexttoward)
#define remainder(value, value2) __TGM_FUNC2(value, value2, remainder)
#define remquo(value, value2, value3) __TGM_FUNC3(value, value2, value3, remquo)
#define rint(value) __TGM_FUNC(value, rint)
#define round(value) __TGM_FUNC(value, round)
#define scalbn(value, value2) __TGM_FUNC2(value, value2, scalbn)
#define scalbln(value, value2) __TGM_FUNC2(value, value2, scalbln)
#define tgamma(value) __TGM_FUNC(value, tgamma)
#define trunc(value) __TGM_FUNC(value, trunc)

#define carg(value) __TGM_FUNC(value, carg)
#define cimag(value) __TGM_FUNC(value, cimag)
#define conj(value) __TGM_FUNC(value, conj)
#define cproj(value) __TGM_FUNC(value, cproj)
#define creal(value) __TGM_FUNC(value, creal)

#endif
