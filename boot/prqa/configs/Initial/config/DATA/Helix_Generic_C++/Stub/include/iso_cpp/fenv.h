
#ifndef _FENV_H
#define _FENV_H

#define FE_INVALID   0x01
#define FE_DIVBYZERO 0x04
#define FE_OVERFLOW	 0x08
#define FE_UNDERFLOW 0x10
#define FE_INEXACT   0x20

#define FE_ALL_EXCEPT (FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)

#define FE_TONEAREST      0
#define FE_DOWNWARD   0x400
#define FE_UPWARD     0x800
#define FE_TOWARDZERO 0xC00

typedef struct {int __opaque;} fenv_t;
typedef unsigned short int fexcept_t;

#define FE_DFL_ENV	((const fenv_t *) -1)

int feclearexcept(int excepts);
int fegetexceptflag(fexcept_t *flagp, int excepts);
int feraiseexcept(int excepts);
int fesetexceptflag(const fexcept_t *flagp, int excepts);
int fetestexcept(int excepts);
int fegetround(void);
int fesetround(int round);
int fegetenv(fenv_t *envp);
int feholdexcept(fenv_t *envp);
int fesetenv(const fenv_t *envp);
int feupdateenv(const fenv_t *envp);


#endif
