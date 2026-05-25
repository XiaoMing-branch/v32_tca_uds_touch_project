#pragma PRQA_MESSAGES_OFF 841,883,1170,3429,3431,3438,3453
#undef assert
#pragma PRQA_MACRO_MESSAGES_OFF "assert"
#ifdef NDEBUG
#define assert(ignore)  ((void)0)
#else
void __abort(void);
#pragma PRQA_NO_RETURN __abort
#define assert(expr)  ((expr)?((void)0):__abort())
#endif

#undef static_assert
#define static_assert _Static_assert
