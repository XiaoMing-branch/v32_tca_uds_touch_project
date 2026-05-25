#pragma PRQA_MESSAGES_OFF 776,777,815,1148,1149,3429,3453,3460
#ifndef _COMPLEX_H
#define _COMPLEX_H

/* Preliminary support for complex numbers. */
/* Although we distingiush the complex and imaginary types, we do not model the complex value completely yet. */

#define imaginary _Imaginary
#define complex _Complex

#define _Imaginary_I ((imaginary float)1.0f) // TODO this is wrong (produces 0i), need support for "1.0fi" syntax
                                             //      not possible to define this constant without extension magic
#define _Complex_I (0.0f + _Imaginary_I)

#define I _Imaginary_I

#pragma PRQA_MACRO_MESSAGES_OFF "_Imaginary_I" 1148,1159,3892
#pragma PRQA_MACRO_MESSAGES_OFF "_Complex_I"   1149,1153,3892

#define CMPLX(x, y) ((double complex)((double)(x) + \
                       _Imaginary_I * (double)(y)))
#define CMPLXF(x, y) ((float complex)((float)(x) + \
                      _Imaginary_I * (float)(y)))
#define CMPLXL(x, y) ((long double complex)((long double)(x) + \
                             _Imaginary_I * (long double)(y)))

#pragma PRQA_MACRO_MESSAGES_OFF "CMPLX"  1148,1149,1155,1156,1159,3212,3390,3395,3401
#pragma PRQA_MACRO_MESSAGES_OFF "CMPLXF" 1148,1149,1155,1156,1159,3212,3390,3395,3401
#pragma PRQA_MACRO_MESSAGES_OFF "CMPLXL" 1148,1149,1155,1156,1159,3212,3390,3395,3401

double complex       cacos(double complex z);
float complex        cacosf(float complex z);
long double complex  cacosl(long double complex z);
double complex       casin(double complex z);
float complex        casinf(float complex z);
long double complex  casinl(long double complex z);
double complex       catan(double complex z);
float complex        catanf(float complex z);
long double complex  catanl(long double complex z);
double complex       ccos(double complex z);
float complex        ccosf(float complex z);
long double complex  ccosl(long double complex z);
double complex       csin(double complex z);
float complex        csinf(float complex z);
long double complex  csinl(long double complex z);
double complex       ctan(double complex z);
float complex        ctanf(float complex z);
long double complex  ctanl(long double complex z);
double complex       cacosh(double complex z);
float complex        cacoshf(float complex z);
long double complex  cacoshl(long double complex z);
double complex       casinh(double complex z);
float complex        casinhf(float complex z);
long double complex  casinhl(long double complex z);
double complex       catanh(double complex z);
float complex        catanhf(float complex z);
long double complex  catanhl(long double complex z);
double complex       ccosh(double complex z);
float complex        ccoshf(float complex z);
long double complex  ccoshl(long double complex z);
double complex       csinh(double complex z);
float complex        csinhf(float complex z);
long double complex  csinhl(long double complex z);
double complex       ctanh(double complex z);
float complex        ctanhf(float complex z);
long double complex  ctanhl(long double complex z);
double complex       cexp(double complex z);
float complex        cexpf(float complex z);
long double complex  cexpl(long double complex z);
double complex       clog(double complex z);
float complex        clogf(float complex z);
long double complex  clogl(long double complex z);
double               cabs(double complex z);
float                cabsf(float complex z);
long double          cabsl(long double complex z);
double complex       cpow(double complex x, double complex y);
float complex        cpowf(float complex x, float complex y);
long double complex  cpowl(long double complex x, long double complex y);
double complex       csqrt(double complex z);
float complex        csqrtf(float complex z);
long double complex  csqrtl(long double complex z);
double               carg(double complex z);
float                cargf(float complex z);
long double          cargl(long double complex z);
double               cimag(double complex z);
float                cimagf(float complex z);
long double          cimagl(long double complex z);
double complex       conj(double complex z);
float complex        conjf(float complex z);
long double complex  conjl(long double complex z);
double complex       cproj(double complex z);
float complex        cprojf(float complex z);
long double complex  cprojl(long double complex z);
double               creal(double complex z);
float                crealf(float complex z);
long double          creall(long double complex z);

#endif
