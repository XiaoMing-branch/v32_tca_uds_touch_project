// qac_wchar_t.h
//

#ifndef INCLUDED_QAC_WCHAR_T_H
#define INCLUDED_QAC_WCHAR_T_H


#if !defined(_WCHAR_T) && defined(PRQA_WCHAR_T)
#define _WCHAR_T
typedef PRQA_WCHAR_T wchar_t;
#endif

#endif
