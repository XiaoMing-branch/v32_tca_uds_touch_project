#pragma PRQA_MESSAGES_OFF 0776,0777
#pragma PRQA_MESSAGES_OFF 1011,1057,1077,1307,1336
#pragma PRQA_MESSAGES_OFF 3313,3412,3414,3602,3672

// suppressions for TLS addendum
#pragma PRQA_MESSAGES_OFF 1335,3429,3435,3436,3453,3456

#ifndef STANDARD_LIBRARY_HEADER_THREADS_IS_INCLUDED
#define STANDARD_LIBRARY_HEADER_THREADS_IS_INCLUDED

// The header <threads.h> includes the header <time.h>, defines macros, and
// declares types, enumeration constants, and functions that support multiple threads of
// execution.

// Implementations that define the macro __STDC_NO_THREADS__ need not provide
// this header nor support any of its facilities.

//~~~~~~~~  Headers  ~~~~~~~~

#include <time.h>

//~~~~~~~~  (Implementation Defined)  ~~~~~~~~

#define THREADS_IMPLEMENTATION_DEFINED_VALUE  42

typedef struct { int Implementation_Defined; } Threads_Implementation_Defined_Type;

//~~~~~~~~  Macros  ~~~~~~~~

// The macros are
//
//        thread_local
//
// which expands to _Thread_local;
//
#define thread_local _Thread_local

//        ONCE_FLAG_INIT
//
// which expands to a value that can be used to initialize an object of type once_flag;
//
#define ONCE_FLAG_INIT  (Threads_Implementation_Defined_Type) { THREADS_IMPLEMENTATION_DEFINED }

// and
//
//        TSS_DTOR_ITERATIONS
//
// which expands to an integer constant expression representing the maximum number of
// times that destructors will be called when a thread terminates.
//
#define TSS_DTOR_ITERATIONS THREADS_IMPLEMENTATION_DEFINED_VALUE

//~~~~~~~~  Types  ~~~~~~~~

// The types are
//
//        cnd_t
//
// which is a complete object type that holds an identifier for a condition variable;
//
typedef Threads_Implementation_Defined_Type cnd_t;

//        thrd_t
//
// which is a complete object type that holds an identifier for a thread;
//
typedef Threads_Implementation_Defined_Type thrd_t;

//        tss_t
//
// which is a complete object type that holds an identifier for a thread-specific storage
// pointer;
//
typedef Threads_Implementation_Defined_Type tss_t;

//        mtx_t
//
// which is a complete object type that holds an identifier for a mutex;
//
typedef Threads_Implementation_Defined_Type mtx_t;

//        tss_dtor_t
//
// which is the function pointer type void (*)(void*), used for a destructor for a
// thread-specific storage pointer;
//
typedef void (* tss_dtor_t) (void *);

//        thrd_start_t
//
// which is the function pointer type int (*)(void*) that is passed to thrd_create
// to create a new thread;
//
typedef int (* thrd_start_t) (void *);

// and
//        once_flag
//
// which is a complete object type that holds a flag for use by call_once.
//
typedef Threads_Implementation_Defined_Type once_flag;

//~~~~~~~~  Enumeration Constants  ~~~~~~~~

// The enumeration constants are
//
enum Threads_Implementation_Defined_Enumeration
{
  //        mtx_plain
  //
  // which is passed to mtx_init to create a mutex object that supports neither timeout nor
  // test and return;
  //
  mtx_plain,

  //        mtx_recursive
  //
  // which is passed to mtx_init to create a mutex object that supports recursive locking;
  //
  mtx_recursive,

  //        mtx_timed
  //
  // which is passed to mtx_init to create a mutex object that supports timeout;
  //
  mtx_timed,

  //        thrd_timedout
  //
  // which is returned by a timed wait function to indicate that the time specified in the call
  // was reached without acquiring the requested resource;
  //
  thrd_timedout,

  //        thrd_success
  //
  // which is returned by a function to indicate that the requested operation succeeded;
  //
  thrd_success,

  //        thrd_busy
  //
  // which is returned by a function to indicate that the requested operation failed because a
  // resource requested by a test and return function is already in use;
  //
  thrd_busy,

  //        thrd_error
  //
  // which is returned by a function to indicate that the requested operation failed;
  //
  thrd_error,

  // and
  //        thrd_nomem
  //
  // which is returned by a function to indicate that the requested operation failed because it
  // was unable to allocate memory.
  //
  thrd_nomem
};

//~~~~~~~~  Functions  ~~~~~~~~

  //~~~~~~~~  Initialization functions  ~~~~~~~~

  // The call_once function uses the once_flag pointed to by flag to ensure that
  // func is called exactly once, the first time the call_once function is called with that
  // value of flag. Completion of an effective call to the call_once function synchronizes
  // with all subsequent calls to the call_once function with the same value of flag.
  //
  void call_once (once_flag * flag,
                  void (* func) (void));

  //~~~~~~~~  Condition variable functions  ~~~~~~~~

  // The cnd_broadcast function unblocks all of the threads that are blocked on the
  // condition variable pointed to by cond at the time of the call. If no threads are blocked
  // on the condition variable pointed to by cond at the time of the call, the function does
  // nothing.
  //
  // The cnd_broadcast function returns thrd_success on success, or thrd_error
  // if the request could not be honored.
  //
  int cnd_broadcast (cnd_t * cond);

  // The cnd_destroy function releases all resources used by the condition variable
  // pointed to by cond. The cnd_destroy function requires that no threads be blocked
  // waiting for the condition variable pointed to by cond.
  //
  void cnd_destroy (cnd_t * cond);

  // The cnd_init function creates a condition variable. If it succeeds it sets the variable
  // pointed to by cond to a value that uniquely identifies the newly created condition
  // variable. A thread that calls cnd_wait on a newly created condition variable will
  // block.
  //
  // The cnd_init function returns thrd_success on success, or thrd_nomem if no
  // memory could be allocated for the newly created condition, or thrd_error if the
  // request could not be honored.
  //
  int cnd_init (cnd_t * cond);

  // The cnd_signal function unblocks one of the threads that are blocked on the
  // condition variable pointed to by cond at the time of the call. If no threads are blocked
  // on the condition variable at the time of the call, the function does nothing and return
  // success.
  //
  // The cnd_signal function returns thrd_success on success or thrd_error if
  // the request could not be honored.
  //
  int cnd_signal (cnd_t * cond);

  // The cnd_timedwait function atomically unlocks the mutex pointed to by mtx and
  // endeavors to block until the condition variable pointed to by cond is signaled by a call to
  // cnd_signal or to cnd_broadcast, or until after the TIME_UTC-based calendar
  // time pointed to by ts. When the calling thread becomes unblocked it locks the variable
  // pointed to by mtx before it returns. The cnd_timedwait function requires that the
  // mutex pointed to by mtx be locked by the calling thread.
  //
  // The cnd_timedwait function returns thrd_success upon success, or
  // thrd_timedout if the time specified in the call was reached without acquiring the
  // requested resource, or thrd_error if the request could not be honored.
  //
  int cnd_timedwait (cnd_t * restrict cond,
                     mtx_t * restrict mtx,
                     const struct timespec * restrict ts);

  // The cnd_wait function atomically unlocks the mutex pointed to by mtx and endeavors
  // to block until the condition variable pointed to by cond is signaled by a call to
  // cnd_signal or to cnd_broadcast. When the calling thread becomes unblocked it
  // locks the mutex pointed to by mtx before it returns. The cnd_wait function requires
  // that the mutex pointed to by mtx be locked by the calling thread.
  //
  // The cnd_wait function returns thrd_success on success or thrd_error if the
  // request could not be honored.
  //
  int cnd_wait (cnd_t * cond, mtx_t * mtx);

  //~~~~~~~~  Mutex functions  ~~~~~~~~

  // The mtx_destroy function releases any resources used by the mutex pointed to by
  // mtx. No threads can be blocked waiting for the mutex pointed to by mtx.
  //
  void mtx_destroy (mtx_t * mtx);

  // The mtx_init function creates a mutex object with properties indicated by type,
  // which must have one of the six values:
  //
  //  mtx_plain                   for a simple non-recursive mutex,
  //  mtx_timed                   for a non-recursive mutex that supports timeout,
  //  mtx_plain | mtx_recursive   for a simple recursive mutex, or
  //  mtx_timed | mtx_recursive   for a recursive mutex that supports timeout.
  //
  // If the mtx_init function succeeds, it sets the mutex pointed to by mtx to a value that
  //  uniquely identifies the newly created mutex.
  //
  //  The mtx_init function returns thrd_success on success, or thrd_error if the
  //  request could not be honored.
  //
  int mtx_init (mtx_t * mtx,
                int     type);

  // The mtx_lock function blocks until it locks the mutex pointed to by mtx. If the mutex
  // is non-recursive, it shall not be locked by the calling thread. Prior calls to mtx_unlock
  // on the same mutex shall synchronize with this operation.
  //
  // The mtx_lock function returns thrd_success on success, or thrd_error if the
  // request could not be honored.
  //
  int mtx_lock (mtx_t * mtx);

  // The mtx_timedlock function endeavors to block until it locks the mutex pointed to by
  // mtx or until after the TIME_UTC-based calendar time pointed to by ts. The specified
  // mutex shall support timeout. If the operation succeeds, prior calls to mtx_unlock on
  // the same mutex shall synchronize with this operation.
  //
  // The mtx_timedlock function returns thrd_success on success, or
  // thrd_timedout if the time specified was reached without acquiring the requested
  // resource, or thrd_error if the request could not be honored.
  //
  int mtx_timedlock (mtx_t * restrict mtx,
                     const struct timespec * restrict ts);

  // The mtx_trylock function endeavors to lock the mutex pointed to by mtx. If the
  // mutex is already locked, the function returns without blocking. If the operation succeeds,
  // prior calls to mtx_unlock on the same mutex shall synchronize with this operation.
  //
  // The mtx_trylock function returns thrd_success on success, or thrd_busy if
  // the resource requested is already in use, or thrd_error if the request could not be
  // honored.
  //
  int mtx_trylock (mtx_t * mtx);

  // The mtx_unlock function unlocks the mutex pointed to by mtx. The mutex pointed to
  // by mtx shall be locked by the calling thread.
  //
  // The mtx_unlock function returns thrd_success on success or thrd_error if
  // the request could not be honored.
  //
  int mtx_unlock (mtx_t * mtx);

  //~~~~~~~~  Thread functions  ~~~~~~~~

  //  The thrd_create function creates a new thread executing func(arg). If the
  //  thrd_create function succeeds, it sets the object pointed to by thr to the identifier of
  //  the newly created thread. (A thread's identifier may be reused for a different thread once
  //  the original thread has exited and either been detached or joined to another thread.) The
  //  completion of the thrd_create function synchronizes with the beginning of the
  //  execution of the new thread.
  //
  //  The thrd_create function returns thrd_success on success, or thrd_nomem if
  //  no memory could be allocated for the thread requested, or thrd_error if the request
  //  could not be honored.
  //
  int thrd_create (thrd_t *      thr,
                   thrd_start_t  func,
                   void *        arg);

  // The thrd_current function identifies the thread that called it.
  //
  // The thrd_current function returns the identifier of the thread that called it.
  //
  thrd_t thrd_current (void);

  // The thrd_detach function tells the operating system to dispose of any resources
  // allocated to the thread identified by thr when that thread terminates. The thread
  // identified by thr shall not have been previously detached or joined with another thread.
  //
  // The thrd_detach function returns thrd_success on success or thrd_error if
  // the request could not be honored.
  //
  int thrd_detach (thrd_t thr);

  // The thrd_equal function will determine whether the thread identified by thr0 refers
  // to the thread identified by thr1.
  //
  // The thrd_equal function returns zero if the thread thr0 and the thread thr1 refer to
  // different threads. Otherwise the thrd_equal function returns a nonzero value.
  //
  int thrd_equal (thrd_t thr0,
                  thrd_t thr1);

  // The thrd_exit function terminates execution of the calling thread and sets its result
  // code to res.
  //
  // The program shall terminate normally after the last thread has been terminated. The
  // behavior shall be as if the program called the exit function with the status
  // EXIT_SUCCESS at thread termination time.
  //
  _Noreturn void thrd_exit (int res);

  // The thrd_join function joins the thread identified by thr with the current thread by
  // blocking until the other thread has terminated. If the parameter res is not a null pointer,
  // it stores the thread's result code in the integer pointed to by res. The termination of the
  // other thread synchronizes with the completion of the thrd_join function. The thread
  // identified by thr shall not have been previously detached or joined with another thread.
  //
  // The thrd_join function returns thrd_success on success or thrd_error if the
  // request could not be honored.
  //
  int thrd_join (thrd_t  thr,
                 int *   res);

  // The thrd_sleep function suspends execution of the calling thread until either the
  // interval specified by duration has elapsed or a signal which is not being ignored is
  // received. If interrupted by a signal and the remaining argument is not null, the
  // amount of time remaining (the requested interval minus the time actually slept) is stored
  // in the interval it points to. The duration and remaining arguments may point to the
  // same object.
  //
  // The suspension time may be longer than requested because the interval is rounded up to
  // an integer multiple of the sleep resolution or because of the scheduling of other activity
  // by the system. But, except for the case of being interrupted by a signal, the suspension
  // time shall not be less than that specified, as measured by the system clock TIME_UTC.
  //
  // The thrd_sleep function returns zero if the requested time has elapsed, -1 if it has
  // been interrupted by a signal, or a negative value if it fails.
  //
  int thrd_sleep (const struct timespec * duration,
                  struct timespec * remaining);

  // The thrd_yield function endeavors to permit other threads to run, even if the current
  // thread would ordinarily continue to run.
  //
  void thrd_yield (void);

  //~~~~~~~~  Thread-specific storage functions  ~~~~~~~~

#if 0   // for reference: this is the original header text which just declares the functions

  // The tss_create function creates a thread-specific storage pointer with destructor
  // dtor, which may be null.
  //
  // If the tss_create function is successful, it sets the thread-specific storage pointed to
  // by key to a value that uniquely identifies the newly created pointer and returns
  // thrd_success; otherwise, thrd_error is returned and the thread-specific storage
  // pointed to by key is set to an undefined value.
  //
  int tss_create (tss_t * key,
                  tss_dtor_t dtor);

  // The tss_delete function releases any resources used by the thread-specific storage
  // identified by key.
  //
  void tss_delete (tss_t key);

  // The tss_get function returns the value for the current thread held in the thread-specific
  // storage identified by key.
  //
  // The tss_get function returns the value for the current thread if successful, or zero if
  // unsuccessful.
  //
  void * tss_get (tss_t key);

  // The tss_set function sets the value for the current thread held in the thread-specific
  // storage identified by key to val.
  //
  // The tss_set function returns thrd_success on success or thrd_error if the
  // request could not be honored.
  //
  int tss_set (tss_t key, void * val);

#else   // interface to Dataflow analysis

#ifndef __QAC_TLS_CREATE // also defined in common_tls

typedef struct { int _i; }  *__qac_tls_key_t;

int __qac_tls_marker (int, ...);

#define __QAC_TLS_CREATE 0
#define __QAC_TLS_DELETE 1
#define __QAC_TLS_SET    2
#define __QAC_TLS_GET    3

#define tss_t                   __qac_tls_key_t

#define tss_create(pkey,dtor)   __qac_tls_marker (__QAC_TLS_CREATE, *(pkey), dtor)

#define tss_delete(key)         __qac_tls_marker (__QAC_TLS_DELETE, key)

#define tss_set(key,value)      __qac_tls_marker (__QAC_TLS_SET, key, value)

#define tss_get(key)            (__qac_tls_marker (__QAC_TLS_GET, key), key)

#endif // __QAC_TLS_CREATE
#endif // USE_DATAFLOW_PLATFORMS_PATH

#endif // not STANDARD_LIBRARY_HEADER_THREADS_IS_INCLUDED
