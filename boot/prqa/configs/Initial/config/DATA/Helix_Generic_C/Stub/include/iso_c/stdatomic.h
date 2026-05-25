
#pragma PRQA_MESSAGES_OFF 776,777,839,847,1011,1027,1030,1056,1081,1300,1307,2038,2052,3110,3412,3417,3429,3435,3443,3453,3456,3457,3600-3632,9999

#ifndef STANDARD_LIBRARY_HEADER_STDATOMIC_IS_INCLUDED
#define STANDARD_LIBRARY_HEADER_STDATOMIC_IS_INCLUDED

// _Atomic is supported.
#ifdef __STDC_NO_ATOMICS__
#undef __STDC_NO_ATOMICS__
#endif

// The header <stdatomic.h> defines several macros and declares several types and
// functions for performing atomic operations on data shared between threads.

// Implementations that define the macro __STDC_NO_ATOMICS__ need not provide
// this header nor support any of its facilities.

//~~~~~~~~  (Implementation Defined)  ~~~~~~~~

#include <stddef.h>   // for wchar_t
#include <uchar.h>    // for char16_t
#include <stdint.h>   // for int_least8_t

//~~~~~~~~  Macros  ~~~~~~~~

// The macros defined are the atomic lock-free macros
//
//    ATOMIC_BOOL_LOCK_FREE
//    ATOMIC_CHAR_LOCK_FREE
//    ATOMIC_CHAR16_T_LOCK_FREE
//    ATOMIC_CHAR32_T_LOCK_FREE
//    ATOMIC_WCHAR_T_LOCK_FREE
//    ATOMIC_SHORT_LOCK_FREE
//    ATOMIC_INT_LOCK_FREE
//    ATOMIC_LONG_LOCK_FREE
//    ATOMIC_LLONG_LOCK_FREE
//    ATOMIC_POINTER_LOCK_FREE
//
// which indicate the lock-free property of the corresponding atomic types (both signed and
// unsigned);
//
#define ATOMIC_BOOL_LOCK_FREE      2
#define ATOMIC_CHAR_LOCK_FREE      2
#define ATOMIC_CHAR16_T_LOCK_FREE  2
#define ATOMIC_CHAR32_T_LOCK_FREE  2
#define ATOMIC_WCHAR_T_LOCK_FREE   2
#define ATOMIC_SHORT_LOCK_FREE     2
#define ATOMIC_INT_LOCK_FREE       2
#define ATOMIC_LONG_LOCK_FREE      2
#define ATOMIC_LLONG_LOCK_FREE     2
#define ATOMIC_POINTER_LOCK_FREE   2

// and
//    ATOMIC_FLAG_INIT
// which expands to an initializer for an object of type atomic_flag.
//
#define ATOMIC_FLAG_INIT { 0 }

//~~~~~~~~  Types  ~~~~~~~~

// The types include
//    memory_order
// which is an enumerated type whose enumerators identify memory ordering constraints;
//
// [see further below for memory_order definition]

//    atomic_flag
// which is a structure type representing a lock-free, primitive atomic flag; and several
// atomic analogs of integer types.
//
typedef struct
{
  int atomic_flag_1;
  int atomic_flag_2;
}  atomic_flag;

// In the following synopses:
// - An A refers to one of the atomic types.
// - A C refers to its corresponding non-atomic type.
// - An M refers to the type of the other argument for arithmetic operations. For atomic
//   integer types, M is C. For atomic pointer types, M is ptrdiff_t.
// - The functions not ending in _explicit have the same semantics as the
//   corresponding _explicit function with memory_order_seq_cst for the
//   memory_order argument.
//
// NOTE Many operations are volatile-qualified. The "volatile as device register" semantics have not
// changed in the standard. This qualification means that volatility is preserved when applying these
// operations to volatile objects.

//~~~~~~~~  Initialization  ~~~~~~~~

// Synopsis
//    #define ATOMIC_VAR_INIT(C value)
//
// The ATOMIC_VAR_INIT macro expands to a token sequence suitable for initializing an
// atomic object of a type that is initialization-compatible with value. An atomic object
// with automatic storage duration that is not explicitly initialized using
// ATOMIC_VAR_INIT is initially in an indeterminate state; however, the default (zero)
// initialization for objects with static or thread-local storage duration is guaranteed to
// produce a valid state.
//
// Concurrent access to the variable being initialized, even via an atomic operation,
// constitutes a data race.
//
// EXAMPLE
//      atomic_int guide = ATOMIC_VAR_INIT(42);
//
#define ATOMIC_VAR_INIT(...) { __VA_ARGS__ } // deprecated in C17, no good implementation

// Synopsis
//    void atomic_init (volatile A * obj,
//                      C            value);
//
// The atomic_init generic function initializes the atomic object pointed to by obj to
// the value value, while also initializing any additional state that the implementation
// might need to carry for the atomic object.
//
// Although this function initializes an atomic object, it does not avoid data races;
// concurrent access to the variable being initialized, even via an atomic operation,
// constitutes a data race.
//
// EXAMPLE
//     atomic_int guide;
//     atomic_init(&guide, 42);
//
#define atomic_init(obj, value) ((void) ((* (obj)) = (value)))

//~~~~~~~~  Order and consistency  ~~~~~~~~

// The enumerated type memory_order specifies the detailed regular (non-atomic)
// memory synchronization operations as defined in 5.1.2.4 and may provide for operation
// ordering. Its enumeration constants are as follows:
//
//    memory_order_relaxed
//    memory_order_consume
//    memory_order_acquire
//    memory_order_release
//    memory_order_acq_rel
//    memory_order_seq_cst
//
// [see C11 standard for full explanation]
//
typedef enum
{
  memory_order_relaxed,
  memory_order_consume,
  memory_order_acquire,
  memory_order_release,
  memory_order_acq_rel,
  memory_order_seq_cst
} memory_order;

// Synopsis
//    type kill_dependency (type y);
//
// The kill_dependency macro terminates a dependency chain; the argument does not
// carry a dependency to the return value.
//
// The kill_dependency macro returns the value of y.
//
#define kill_dependency(y)  (y)

//~~~~~~~~  Fences  ~~~~~~~~

// Synopsis
//    void atomic_thread_fence (memory_order order);
//
// Depending on the value of order, this operation:
// - has no effects, if order == memory_order_relaxed;
// - is an acquire fence, if order == memory_order_acquire or order ==
//   memory_order_consume;
// - is a release fence, if order == memory_order_release;
// - is both an acquire fence and a release fence, if order ==
//   memory_order_acq_rel;
// - is a sequentially consistent acquire and release fence, if order ==
//   memory_order_seq_cst.
//
void atomic_thread_fence (memory_order order);

// Synopsis
//    void atomic_signal_fence (memory_order order);
//
// Equivalent to atomic_thread_fence(order), except that the resulting ordering
// constraints are established only between a thread and a signal handler executed in the
// same thread.
//
// NOTE 1 The atomic_signal_fence function can be used to specify the order in which actions
// performed by the thread become visible to the signal handler.
//
// NOTE 2 Compiler optimizations and reorderings of loads and stores are inhibited in the same way as with
// atomic_thread_fence, but the hardware fence instructions that atomic_thread_fence would
// have inserted are not emitted.
//
void atomic_signal_fence (memory_order order);

//~~~~~~~~  Lock-free property  ~~~~~~~~

// Synopsis
//    _Bool atomic_is_lock_free (const volatile A * obj);
//
// The atomic_is_lock_free generic function indicates whether or not the object
// pointed to by obj is lock-free.
//
// The atomic_is_lock_free generic function returns nonzero (true) if and only if
// atomic operations on objects of the type pointed to by the argument are lock-free.
// In any given program execution, the result of the lock-free query shall be
// consistent for all pointers of the same type.
//
// NOTE this behaviour was changed by DR-465
//
#define atomic_is_lock_free(AP) (_Generic ((AP), \
  bool      : ATOMIC_BOOL_LOCK_FREE,             \
  char      : ATOMIC_CHAR_LOCK_FREE,             \
  char16_t  : ATOMIC_CHAR16_T_LOCK_FREE,         \
  char32_t  : ATOMIC_CHAR32_T_LOCK_FREE,         \
  wchar_t   : ATOMIC_WCHAR_T_LOCK_FREE,          \
  short     : ATOMIC_SHORT_LOCK_FREE,            \
  int       : ATOMIC_INT_LOCK_FREE,              \
  long      : ATOMIC_LONG_LOCK_FREE,             \
  long long : ATOMIC_LLONG_LOCK_FREE,            \
  /* ATOMIC_POINTER_LOCK_FREE */                 \
  default   : atomic_is_lock_free (AP)) != 0)
_Bool (atomic_is_lock_free) (void const volatile _Atomic * obj);

//~~~~~~~~  Atomic integer types  ~~~~~~~~

// For each line in the following table, the atomic type name is declared as a type that
// has the same representation and alignment requirements as the corresponding direct
// type.
//
// NOTE The representation of atomic integer types need not have the same size as their corresponding
// regular types. They should have the same size whenever possible, as it eases effort required to port existing
// code.
//
//        Atomic type name        Direct type
//      --------------------    ------------------
//        atomic_bool             _Atomic _Bool
//        atomic_char             _Atomic char
//        atomic_schar            _Atomic signed char
//        atomic_uchar            _Atomic unsigned char
//        atomic_short            _Atomic short
//        atomic_ushort           _Atomic unsigned short
//        atomic_int              _Atomic int
//        atomic_uint             _Atomic unsigned int
//        atomic_long             _Atomic long
//        atomic_ulong            _Atomic unsigned long
//        atomic_llong            _Atomic long long
//        atomic_ullong           _Atomic unsigned long long
//        atomic_char16_t         _Atomic char16_t
//        atomic_char32_t         _Atomic char32_t
//        atomic_wchar_t          _Atomic wchar_t
//        atomic_int_least8_t     _Atomic int_least8_t
//        atomic_uint_least8_t    _Atomic uint_least8_t
//        atomic_int_least16_t    _Atomic int_least16_t
//        atomic_uint_least16_t   _Atomic uint_least16_t
//        atomic_int_least32_t    _Atomic int_least32_t
//        atomic_uint_least32_t   _Atomic uint_least32_t
//        atomic_int_least64_t    _Atomic int_least64_t
//        atomic_uint_least64_t   _Atomic uint_least64_t
//        atomic_int_fast8_t      _Atomic int_fast8_t
//        atomic_uint_fast8_t     _Atomic uint_fast8_t
//        atomic_int_fast16_t     _Atomic int_fast16_t
//        atomic_uint_fast16_t    _Atomic uint_fast16_t
//        atomic_int_fast32_t     _Atomic int_fast32_t
//        atomic_uint_fast32_t    _Atomic uint_fast32_t
//        atomic_int_fast64_t     _Atomic int_fast64_t
//        atomic_uint_fast64_t    _Atomic uint_fast64_t
//        atomic_intptr_t         _Atomic intptr_t
//        atomic_uintptr_t        _Atomic uintptr_t
//        atomic_size_t           _Atomic size_t
//        atomic_ptrdiff_t        _Atomic ptrdiff_t
//        atomic_intmax_t         _Atomic intmax_t
//        atomic_uintmax_t        _Atomic uintmax_t
//
typedef _Atomic (_Bool)                 atomic_bool;
typedef _Atomic (char)                  atomic_char;
typedef _Atomic (signed char)           atomic_schar;
typedef _Atomic (unsigned char)         atomic_uchar;
typedef _Atomic (short)                 atomic_short;
typedef _Atomic (unsigned short)        atomic_ushort;
typedef _Atomic (int)                   atomic_int;
typedef _Atomic (unsigned int)          atomic_uint;
typedef _Atomic (long)                  atomic_long;
typedef _Atomic (unsigned long)         atomic_ulong;
typedef _Atomic (long long)             atomic_llong;
typedef _Atomic (unsigned long long)    atomic_ullong;
typedef _Atomic (char16_t)              atomic_char16_t;
typedef _Atomic (char32_t)              atomic_char32_t;
typedef _Atomic (wchar_t)               atomic_wchar_t;
typedef _Atomic (int_least8_t)          atomic_int_least8_t;
typedef _Atomic (uint_least8_t)         atomic_uint_least8_t;
typedef _Atomic (int_least16_t)         atomic_int_least16_t;
typedef _Atomic (uint_least16_t)        atomic_uint_least16_t;
typedef _Atomic (int_least32_t)         atomic_int_least32_t;
typedef _Atomic (uint_least32_t)        atomic_uint_least32_t;
typedef _Atomic (int_least64_t)         atomic_int_least64_t;
typedef _Atomic (uint_least64_t)        atomic_uint_least64_t;
typedef _Atomic (int_fast8_t)           atomic_int_fast8_t;
typedef _Atomic (uint_fast8_t)          atomic_uint_fast8_t;
typedef _Atomic (int_fast16_t)          atomic_int_fast16_t;
typedef _Atomic (uint_fast16_t)         atomic_uint_fast16_t;
typedef _Atomic (int_fast32_t)          atomic_int_fast32_t;
typedef _Atomic (uint_fast32_t)         atomic_uint_fast32_t;
typedef _Atomic (int_fast64_t)          atomic_int_fast64_t;
typedef _Atomic (uint_fast64_t)         atomic_uint_fast64_t;
typedef _Atomic (intptr_t)              atomic_intptr_t;
typedef _Atomic (uintptr_t)             atomic_uintptr_t;
typedef _Atomic (size_t)                atomic_size_t;
typedef _Atomic (ptrdiff_t)             atomic_ptrdiff_t;
typedef _Atomic (intmax_t)              atomic_intmax_t;
typedef _Atomic (uintmax_t)             atomic_uintmax_t;

//~~~~~~~~  Operations on atomic types  ~~~~~~~~

// Synopsis
//    void atomic_store (volatile A * object,
//                       C            desired);
//
#define atomic_store(object, desired) atomic_store_explicit ((object), (desired), memory_order_seq_cst)

// Synopsis
//    void atomic_store_explicit (volatile A * object,
//                                C            desired,
//                                memory_order order);
//
// The order argument shall not be memory_order_acquire,
// memory_order_consume, nor memory_order_acq_rel. Atomically replace the
// value pointed to by object with the value of desired. Memory is affected according
//  to the value of order.
//
#define atomic_store_explicit(object, desired, order) (void) (* (object) = (desired), order)

// Synopsis
//    C atomic_load (volatile A * object);
//
// Atomically returns the value pointed to by object.
//
#define atomic_load(object) (* (object))

// Synopsis
//    C atomic_load_explicit (volatile A * object,
//                            memory_order order);
//
// The order argument shall not be memory_order_release nor
// memory_order_acq_rel. Memory is affected according to the value of order.
//
// Atomically returns the value pointed to by object.
//
#define atomic_load_explicit(object, order) ((order), * (object))

// Synopsis
//    C atomic_exchange (volatile A * object,
//                       C            desired);
//
// Atomically replace the value pointed to by object with desired.
// These operations are read-modify-write operations
//
// Atomically returns the value pointed to by object immediately before the effects.
//
#define atomic_exchange(object, desired) (*(object) = (desired))

// Synopsis
//    C atomic_exchange_explicit (volatile A * object,
//                                C            desired,
//                                memory_order order);
//
// Atomically replace the value pointed to by object with desired. Memory is affected
// according to the value of order. These operations are read-modify-write operations
//
// Atomically returns the value pointed to by object immediately before the effects.
//
#define atomic_exchange_explicit(object, desired, order) ((order), * (object) = (desired))

// Synopsis
//    _Bool atomic_compare_exchange_strong (volatile A * object,
//                                          C *          expected,
//                                          C            desired);
//
// ...
//
#define atomic_compare_exchange_strong(object, expected, desired) (*  (object), * (expected) == (desired))

// Synopsis
//    _Bool atomic_compare_exchange_strong_explicit (volatile A * object,
//                                                   C *          expected,
//                                                   C            desired,
//                                                   memory_order success,
//                                                   memory_order failure);
//
// ...
//
#define atomic_compare_exchange_strong_explicit(object, expected, desired, success, failure)  ((success) == (failure), *  (object), * (expected) == (desired))

// Synopsis
//    _Bool atomic_compare_exchange_weak (volatile A * object,
//                                        C *          expected,
//                                        C            desired);
//
// ...
//
#define atomic_compare_exchange_weak(object, expected, desired) (*  (object), * (expected) == (desired))

// Synopsis
//    _Bool atomic_compare_exchange_weak_explicit (volatile A * object,
//                                                 C *          expected,
//                                                 C            desired,
//                                                 memory_order success,
//                                                 memory_order failure);
//
// ...
//
#define atomic_compare_exchange_weak_explicit(object, expected, desired, success, failure)  ((success) == (failure), *  (object), * (expected) == (desired))

// Synopsis
//    C atomic_fetch_key (volatile A * object,
//                        M            operand);
//
// ...
//
#define atomic_fetch_add(object, operand) (*(object) += (operand))
#define atomic_fetch_sub(object, operand) (*(object) -= (operand))
#define atomic_fetch_or(object, operand)  (*(object) |= (operand))
#define atomic_fetch_xor(object, operand) (*(object) ^= (operand))
#define atomic_fetch_and(object, operand) (*(object) &= (operand))

// Synopsis
//    C atomic_fetch_key_explicit (volatile A * object,
//                                 M            operand,
//                                 memory_order order);
//
// ...
//
#define atomic_fetch_add_explicit(object, operand, order) ((order), *(object) += (operand))
#define atomic_fetch_sub_explicit(object, operand, order) ((order), *(object) -= (operand))
#define atomic_fetch_or_explicit(object, operand, order)  ((order), *(object) |= (operand))
#define atomic_fetch_xor_explicit(object, operand, order) ((order), *(object) ^= (operand))
#define atomic_fetch_and_explicit(object, operand, order) ((order), *(object) &= (operand))

// Synopsis
//    _Bool atomic_flag_test_and_set (volatile atomic_flag * object);
//
// ...
//
_Bool atomic_flag_test_and_set (volatile atomic_flag * object);

// Synopsis
//    _Bool atomic_flag_test_and_set_explicit (volatile atomic_flag * object,
//                                             memory_order           order);
//
// ...
//
_Bool atomic_flag_test_and_set_explicit (volatile atomic_flag *  object,
                                         memory_order            order);

// Synopsis
//    void atomic_flag_clear (volatile atomic_flag * object);
//
// ...
//
void atomic_flag_clear (volatile atomic_flag * object);

// Synopsis
//    void atomic_flag_clear_explicit (volatile atomic_flag * object,
//                                     memory_order           order);
//
// ...
//
void atomic_flag_clear_explicit (volatile atomic_flag *  object,
                                 memory_order            order);

#endif // not STANDARD_LIBRARY_HEADER_STDATOMIC_IS_INCLUDED
