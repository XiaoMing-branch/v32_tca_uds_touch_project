// paulfitz Thu Jun  3 09:39:18 IST 1999

// BEGIN BLOCK prefix
// Updated by paul on Tue Jun  8 11:15:38 IST 1999
// Part of a simple implementation of the standard C++ library, as
// described in ISO/IEC 14882:1998(E)

#ifndef __QAC_INCLUDE_qac_implement_h__
#define __QAC_INCLUDE_qac_implement_h__

// Some flags to control how closely the standard is followed
#include <qac_config.h>

// END BLOCK prefix

// SPEC CHOICE -- used by <ios> <char_traits.h>
// get a problem with filebuf.cc if these are long int
#define __QAC_TYPE_OFF_T  int
#define __QAC_TYPE_SZ_T int

// SPEC CHOICE -- used by <ios>
// these would be better as enums, as in cygnus implementation
#define __QAC_TYPE_T1 int
enum {__QAC_TYPE_T1_BOOLALPHA,
      __QAC_TYPE_T1_DEC,
      __QAC_TYPE_T1_FIXED,
      __QAC_TYPE_T1_HEX,
      __QAC_TYPE_T1_INTERNAL,
      __QAC_TYPE_T1_LEFT,
      __QAC_TYPE_T1_OCT,
      __QAC_TYPE_T1_RIGHT,
      __QAC_TYPE_T1_SCIENTIFIC,
      __QAC_TYPE_T1_SHOWBASE,
      __QAC_TYPE_T1_SHOWPOINT,
      __QAC_TYPE_T1_SHOWPOS,
      __QAC_TYPE_T1_SKIPWS,
      __QAC_TYPE_T1_UNITBUF,
      __QAC_TYPE_T1_UPPERCASE,
      __QAC_TYPE_T1_ADJUSTFIELD,
      __QAC_TYPE_T1_BASEFIELD,
      __QAC_TYPE_T1_FLOATFIELD
};

#define __QAC_TYPE_T2 int
enum {__QAC_TYPE_T2_BADBIT,
      __QAC_TYPE_T2_EOFBIT,
      __QAC_TYPE_T2_FAILBIT,
      __QAC_TYPE_T2_GOODBIT
};

#define __QAC_TYPE_T3 int
enum {__QAC_TYPE_T3_APP,
      __QAC_TYPE_T3_ATE,
      __QAC_TYPE_T3_BINARY,
      __QAC_TYPE_T3_IN,
      __QAC_TYPE_T3_OUT,
      __QAC_TYPE_T3_TRUNC
};

#define __QAC_TYPE_T4 int
enum {__QAC_TYPE_T4_BEG,
      __QAC_TYPE_T4_CUR,
      __QAC_TYPE_T4_END
};

// BEGIN BLOCK postfix
// Updated by paul on Tue Jun  8 11:15:38 IST 1999
#endif // ends __QAC_INCLUDE_qac_implement_h__
// END BLOCK postfix

