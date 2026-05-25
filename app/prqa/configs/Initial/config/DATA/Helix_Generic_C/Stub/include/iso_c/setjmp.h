#pragma PRQA_MESSAGES_OFF 1077,3132,3429,3453
#ifndef _SETJMP_H
#define _SETJMP_H
typedef int jmp_buf[16];

#define setjmp(env) __setjmp(env)

extern int  __setjmp(jmp_buf env);
extern _Noreturn void longjmp(jmp_buf env, int val);

#endif
