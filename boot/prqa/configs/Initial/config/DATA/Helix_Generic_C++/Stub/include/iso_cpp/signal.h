/* signal.h	lastmod 19 Nov 91  SAC
 *			created 18 Dec 87  SAC
 * version:	@(#)signal.h	1.3
 * date:		95/03/02
 */
#ifndef _SIGNAL_H
#define _SIGNAL_H

typedef int sig_atomic_t;

#define SIG_DFL	(&__sigDefault)
#define SIG_ERR	(&__sigError)
#define SIG_IGN	(&__sigIgnore)

#define SIGHUP       1
#define SIGINT       2
#define SIGQUIT      3
#define SIGILL       4
#define SIGABRT      6
#define SIGBUS       7
#define SIGFPE       8
#define SIGKILL      9
#define SIGUSR1     10
#define SIGSEGV     11
#define SIGUSR2     12
#define SIGPIPE     13
#define SIGALRM     14
#define SIGTERM     15
#define SIGURG      16
#define SIGSTOP     17
#define SIGTSTP     18
#define SIGCONT     19
#define SIGCHLD     20
#define SIGTTIN     21
#define SIGTTOU     22
#define SIGIO       23
#define SIGXCPU     24
#define SIGXFSZ     25
#define SIGVTALRM   26
#define SIGPROF     27
#define SIGWINCH    28
#define SIGPWR      29

#define WNOHANG 1
#define WUNTRACED 2
#define WCONTINUED 4
#define WIFEXITED 8
#define WEXITSTATUS 16
#define WIFSIGNALED 32
#define WTERMSIG 64
#define WCOREDUMP 128
#define WIFSTOPPED 256
#define WSTOPSIG 512
#define WIFCONTINUED 1024

#ifndef __cplusplus
extern void	(*signal(int,void(*)(int)))(int);
#else
extern "C"
{
  typedef int errno_t;
  typedef int pid_t;
  struct sigset_t {};
  struct siginfo_t {};
  struct sigaction {
    void     (*sa_handler)(int);
    void     (*sa_sigaction)(int, siginfo_t *, void *);
    sigset_t   sa_mask;
    int        sa_flags;
    void     (*sa_restorer)(void);
  };

  extern int sigemptyset(sigset_t *set);
  extern int sigfillset(sigset_t *set);
  extern int sigaddset(sigset_t *set, int signum);
  extern int sigdelset(sigset_t *set, int signum);
  extern int sigismember(const sigset_t *set, int signum);
  extern int sigpending(sigset_t *set);

  extern int sigaction(int signum, const struct sigaction *act,
      struct sigaction *oldact);

  extern void	(*signal(int,void(*)(int)))(int);
  extern void __sigError(int);
  extern void __sigDefault(int);
  extern void __sigIgnore(int);
  extern pid_t waitpid(pid_t pid, int *stat_loc, int options);
}
#endif
extern int	raise(int);
#endif
