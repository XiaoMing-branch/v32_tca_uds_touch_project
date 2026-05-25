// paulfitz Thu Jun  3 09:39:18 IST 1999

// BEGIN BLOCK prefix
// Updated by paul on Tue Jun  8 11:15:39 IST 1999
// Part of a simple implementation of the standard C++ library, as
// described in ISO/IEC 14882:1998(E)

#ifndef __QAC_INCLUDE_qac_iterator_h__
#define __QAC_INCLUDE_qac_iterator_h__

// Some flags to control how closely the standard is followed
#include <qac_config.h>

// END BLOCK prefix

// The following is based on code in the STL library (stl_iterator.h)
/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 1996-1998
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */


#include <iterator>


__QAC_BEGIN_NAMESPACE

template<typename _Iterator, typename _Container>
class __normal_iterator
  : public iterator<
                    typename iterator_traits<_Iterator>::iterator_category,
                    typename iterator_traits<_Iterator>::value_type,
                    typename iterator_traits<_Iterator>::difference_type,
                    typename iterator_traits<_Iterator>::pointer,
                    typename iterator_traits<_Iterator>::reference>
{
public:

  typedef __normal_iterator<_Iterator, _Container> normal_iterator_type;

  inline __normal_iterator() {} 
  inline explicit __normal_iterator(const _Iterator& __i) {}

  // Allow iterator to const_iterator conversion
  template<typename _Iter>
  inline __normal_iterator(const __normal_iterator<_Iter, _Container>& __i) { }

  typedef iterator<
                    typename iterator_traits<_Iterator>::iterator_category,
                    typename iterator_traits<_Iterator>::value_type,
                    typename iterator_traits<_Iterator>::difference_type,
                    typename iterator_traits<_Iterator>::pointer,
                    typename iterator_traits<_Iterator>::reference>
	  Qualifier;


  // forward iterator requirements

  inline typename Qualifier::reference
  operator*() const { return *(operator-> ()); }

  inline typename Qualifier::pointer
  operator->() const { static typename Qualifier::pointer * volatile r; return * r;  }

  inline normal_iterator_type&
  operator++() { return *this; }

  inline normal_iterator_type
  operator++(int) { return *this; }

  // bidirectional iterator requirements

  inline normal_iterator_type&
  operator--() { return *this; }

  inline normal_iterator_type
  operator--(int) { return *this; }

  // random access iterator requirements

  inline typename Qualifier::reference
  operator[](typename Qualifier::difference_type __n) const { return (operator*()); }

  inline normal_iterator_type&
  operator+=(typename Qualifier::difference_type __n) { return *this; }

  inline normal_iterator_type
  operator+(typename Qualifier::difference_type __n) const { return *this; }

  inline normal_iterator_type&
  operator-=(typename Qualifier::difference_type __n) { return *this; }
  
  inline normal_iterator_type
  operator-(typename Qualifier::difference_type __n) const { return *this; }

  inline typename Qualifier::difference_type
  operator-(const normal_iterator_type& __i) const { static typename Qualifier::difference_type * volatile r; return * r;  }

  const _Iterator& base() const { static _Iterator  * volatile r; return * r; } 
};

// forward iterator requirements

template<typename _IteratorL, typename _IteratorR, typename _Container>
bool operator==(const __normal_iterator<_IteratorL, _Container>& __lhs,
    const __normal_iterator<_IteratorR, _Container>& __rhs);

template<typename _IteratorL, typename _IteratorR, typename _Container>
bool operator!=(const __normal_iterator<_IteratorL, _Container>& __lhs,
    const __normal_iterator<_IteratorR, _Container>& __rhs);

// random access iterator requirements

template<typename _IteratorL, typename _IteratorR, typename _Container>
bool operator<(const __normal_iterator<_IteratorL, _Container>& __lhs,
    const __normal_iterator<_IteratorR, _Container>& __rhs);

template<typename _IteratorL, typename _IteratorR, typename _Container>
bool operator>(const __normal_iterator<_IteratorL, _Container>& __lhs,
    const __normal_iterator<_IteratorR, _Container>& __rhs);

template<typename _IteratorL, typename _IteratorR, typename _Container>
bool operator<=(const __normal_iterator<_IteratorL, _Container>& __lhs,
    const __normal_iterator<_IteratorR, _Container>& __rhs);

template<typename _IteratorL, typename _IteratorR, typename _Container>
bool operator>=(const __normal_iterator<_IteratorL, _Container>& __lhs,
    const __normal_iterator<_IteratorR, _Container>& __rhs);

template<typename _Iterator, typename _Container>
inline __normal_iterator<_Iterator, _Container>
operator+(typename __normal_iterator<_Iterator, _Container>::Qualifier::difference_type __n,
          const __normal_iterator<_Iterator, _Container>& __i) { return __normal_iterator<_Iterator, _Container>(); } 


__QAC_END_NAMESPACE

// BEGIN BLOCK postfix
// Updated by paul on Tue Jun  8 11:15:39 IST 1999
#endif // ends __QAC_INCLUDE_qac_iterator_h__
// END BLOCK postfix
