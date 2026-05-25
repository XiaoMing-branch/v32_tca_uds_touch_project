// paulfitz Thu Jun  3 09:39:18 IST 1999

// BEGIN BLOCK prefix
// Updated by paul on Thu Aug 12 14:21:06 IST 1999
// Part of a simple implementation of the standard C++ library, as
// described in ISO/IEC 14882:1998(E)

#ifndef __QAC_INCLUDE_char_traits_h__
#define __QAC_INCLUDE_char_traits_h__

// Some flags to control how closely the standard is followed
#include <qac_config.h>

// END BLOCK prefix

#include <qac_implement.h>
#include <cwchar>
#include <cstddef>


__QAC_BEGIN_NAMESPACE

  typedef __QAC_TYPE_OFF_T  streamoff;
  typedef __QAC_TYPE_SZ_T streamsize;
  typedef __QAC_TYPE_OFF_T  wstreamoff;
  typedef __QAC_TYPE_SZ_T wstreamsize;

  template <class stateT> class fpos {
  public:
    //  lib.fpos.members Members
    stateT state() const;
    void state(stateT);

    fpos();
    fpos(streamoff o);

    operator streamoff ();
    bool operator==(const fpos& p) const;
    bool operator!=(const fpos& p) const;
    fpos& operator+=(streamoff o);
    fpos& operator-=(streamoff o);
    fpos operator+(streamoff o) const;
    fpos operator-(streamoff o) const;
    streamoff operator-(const fpos& p) const;
  };

  typedef fpos<mbstate_t> streampos;
  typedef fpos<mbstate_t> wstreampos;


  template<class charT>
  struct char_traits {
    typedef charT       char_type;
    typedef charT       int_type;
    typedef streamoff   off_type;
    typedef streampos   pos_type;
    typedef mbstate_t   state_type;

    static void assign(char_type& c1, const char_type& c2);
    static bool eq(const char_type& c1, const char_type& c2);
    static bool lt(const char_type& c1, const char_type& c2);

    static int compare(const char_type* s1, const char_type* s2, size_t n);
    static size_t length(const char_type* s);
    static const char_type* find(const char_type* s, size_t n,
				 const char_type& a);
    static char_type* move(char_type* s1, const char_type* s2, size_t n);
    static char_type* copy(char_type* s1, const char_type* s2, size_t n);
    static char_type* assign(char_type* s, size_t n, char_type a);

    static int_type not_eof(const int_type& c);
    static char_type to_char_type(const int_type& c);
    static int_type to_int_type(const char_type& c);
    static bool eq_int_type(const int_type& c1, const int_type& c2);
    static int_type eof();
  };


  template<>
  struct char_traits<char> {
    typedef char        char_type;
    typedef int         int_type;
    typedef streamoff   off_type;
    typedef streampos   pos_type;
    typedef mbstate_t   state_type;

    static void assign(char_type& c1, const char_type& c2);
    static bool eq(const char_type& c1, const char_type& c2);
    static bool lt(const char_type& c1, const char_type& c2);

    static int compare(const char_type* s1, const char_type* s2, size_t n);
    static size_t length(const char_type* s);
    static const char_type* find(const char_type* s, size_t n,
				 const char_type& a);
    static char_type* move(char_type* s1, const char_type* s2, size_t n);
    static char_type* copy(char_type* s1, const char_type* s2, size_t n);
    static char_type* assign(char_type* s, size_t n, char_type a);

    static int_type not_eof(const int_type& c);
    static char_type to_char_type(const int_type& c);
    static int_type to_int_type(const char_type& c);
    static bool eq_int_type(const int_type& c1, const int_type& c2);
    static int_type eof();
  };


  template<>
  struct char_traits<wchar_t> {
    typedef wchar_t      char_type;
    typedef wint_t       int_type;
    typedef streamoff   off_type;
    typedef wstreampos   pos_type;
    typedef mbstate_t    state_type;

    static void assign(char_type& c1, const char_type& c2);
    static bool eq(const char_type& c1, const char_type& c2);
    static bool lt(const char_type& c1, const char_type& c2);

    static int compare(const char_type* s1, const char_type* s2, size_t n);
    static size_t length(const char_type* s);
    static const char_type* find(const char_type* s, size_t n,
				 const char_type& a);
    static char_type* move(char_type* s1, const char_type* s2, size_t n);
    static char_type* copy(char_type* s1, const char_type* s2, size_t n);
    static char_type* assign(char_type* s, size_t n, char_type a);

    static int_type not_eof(const int_type& c);
    static char_type to_char_type(const int_type& c);
    static int_type to_int_type(const char_type& c);
    static bool eq_int_type(const int_type& c1, const int_type& c2);
    static int_type eof();
  };


__QAC_END_NAMESPACE


// BEGIN BLOCK postfix
// Updated by paul on Tue Jun  8 11:15:37 IST 1999
#endif // ends __QAC_INCLUDE_char_traits_h__
// END BLOCK postfix

