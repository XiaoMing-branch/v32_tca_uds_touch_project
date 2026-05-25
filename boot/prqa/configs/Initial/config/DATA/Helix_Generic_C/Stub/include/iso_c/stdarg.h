#pragma PRQA_MESSAGES_OFF 0850,1011,1030,1252,1278,2017,3332,3410,3429,3453,3619
#ifndef included_STDARG_H
#define included_STDARG_H
#ifndef _VA_LIST
#define _VA_LIST
typedef void * va_list;
#endif


// as of C23 va_start is NOT ALLOWED to use the
// second argument for ay purpose, needs to be implicit
#if __STDC_VERSION__ >= 202212L // ("after 2022" -> C23)
  void * __prqa_va_start_address (void);

  // NOTE as of C23 calling this with one arg is well-defined (have __VA_OPT__)
  //      although it does accept the second arg unused for historical reasons
  #define __prqa_va_start(ap, ...) ((void)((ap) = __prqa_va_start_address ()))
#else
  #define __prqa_va_start(ap, parmN)  ((void)((ap)= &(parmN)))
#endif

#define va_start(...) __prqa_va_start (__VA_ARGS__) // handle one or two


/*
 * NOTE: va_arg doesn't actually *do* anything, but that doesn't matter for
 * static analysis!
 */
#define va_arg(ap, type)  (*(type*)(ap))
#define va_copy(ap, src)  ((void)((ap)=(src)))
#define va_end(ap)        ((void)((ap)=(void *)0))

#endif
