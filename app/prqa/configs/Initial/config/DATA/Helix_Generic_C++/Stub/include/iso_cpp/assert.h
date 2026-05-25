/* assert.h */

#ifdef assert
#undef assert
#endif

#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
#define assert(expr) __assert(expr,#expr,__FILE__,__LINE__)
extern void __assert(bool,char const*,char const*,int);
#endif
