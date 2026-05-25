#pragma PRQA_MESSAGES_OFF 631,750,780,782,1307,1336,3602,3630,3672

#ifndef _SIGNAL_H
#define _SIGNAL_H

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

#ifdef __cplusplus
extern "C"
{
#endif
  typedef int errno_t;
  typedef int uid_t;
  typedef int pid_t;
  typedef int sigset_t;
  typedef int sig_atomic_t;

  union sigval {
    int sival_int;
    void *sival_ptr;
  };

  typedef struct {
    int si_signo;
    int si_code;
    union sigval si_value;
    int si_errno;
    pid_t si_pid;
    uid_t si_uid;
    void *si_addr;
    int si_status;
    int si_band;
  } siginfo_t;

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

  extern void (*signal(int signum,void(*handler)(int)))(int);
  extern int raise(int signum);
  extern pid_t waitpid(pid_t pid, int *stat_loc, int options);
#ifdef __cplusplus
}
#endif
#endif
