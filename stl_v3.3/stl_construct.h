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
 * Copyright (c) 1996,1997
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

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#ifndef __SGI_STL_INTERNAL_CONSTRUCT_H
#define __SGI_STL_INTERNAL_CONSTRUCT_H

#include <new.h>

__STL_BEGIN_NAMESPACE

// construct and destroy.  These functions are not part of the C++ standard,
// and are provided for backward compatibility with the HP STL.  We also
// provide internal names _Construct and _Destroy that can be used within
// the library, so that standard-conforming pieces don't have to rely on
// non-standard extensions.

// Internal names


// http://www.cnblogs.com/luxiaoxun/archive/2012/08/10/2631812.html
// new ：不能被重载，其行为总是一致的。
// 它先调用operator new分配内存，然后调用构造函数初始化那段内存。

// new 操作符的执行过程：
// 1. 调用operator new分配内存 ；
// 2. 调用构造函数生成类对象；
// 3. 返回相应指针。

// （2）operator new：要实现不同的内存分配行为，应该重载operator new，而不是new。

// operator new就像operator + 一样，是可以重载的。
// 如果类中没有重载operator new，那么调用的就是全局的::operator new来完成堆的分配。
// 同理，operator new[]、operator delete、operator delete[]也是可以重载的。

// （3）placement new：只是operator new重载的一个版本。它并不分配内存，
// 只是返回指向已经分配好的某段内存的一个指针。
// 因此不能删除它，但需要调用对象的析构函数。

// 如果你想在已经分配的内存中创建一个对象，使用new时行不通的。
// 也就是说placement new允许你在一个已经分配好的内存中（栈或者堆中）构造一个新的对象。
// 原型中void* p实际上就是指向一个已经分配好的内存缓冲区的的首地址。


// placement new
template <class _T1, class _T2>
inline void _Construct(_T1* __p, const _T2& __value) {
  new ((void*) __p) _T1(__value);
}

// new时候，先传入了指针，然后再调用构造函数
// 类似于 new p A();我曹，A()是构造函数，不是类型名
// new A;这样A是类型名。
template <class _T1>
inline void _Construct(_T1* __p) {
  new ((void*) __p) _T1();
}

// placement new
// 需要直接调用析构函数去析构.
// 因为内存空间不是placement new分配的,所以不建议使用delete释放空间.
template <class _Tp>
inline void _Destroy(_Tp* __pointer) {
  __pointer->~_Tp();
}

// 批量释放
template <class _ForwardIterator>
void
__destroy_aux(_ForwardIterator __first, _ForwardIterator __last, __false_type)
{
  for ( ; __first != __last; ++__first)
    destroy(&*__first);
}

template <class _ForwardIterator> 
inline void __destroy_aux(_ForwardIterator, _ForwardIterator, __true_type) {}

template <class _ForwardIterator, class _Tp>
inline void 
__destroy(_ForwardIterator __first, _ForwardIterator __last, _Tp*)
{
  typedef typename __type_traits<_Tp>::has_trivial_destructor
          _Trivial_destructor;
  __destroy_aux(__first, __last, _Trivial_destructor());
}

template <class _ForwardIterator>
inline void _Destroy(_ForwardIterator __first, _ForwardIterator __last) {
  __destroy(__first, __last, __VALUE_TYPE(__first));
}

// 特例化
inline void _Destroy(char*, char*) {}
inline void _Destroy(int*, int*) {}
inline void _Destroy(long*, long*) {}
inline void _Destroy(float*, float*) {}
inline void _Destroy(double*, double*) {}
#ifdef __STL_HAS_WCHAR_T
inline void _Destroy(wchar_t*, wchar_t*) {}
#endif /* __STL_HAS_WCHAR_T */

// --------------------------------------------------
// Old names from the HP STL.

template <class _T1, class _T2>
inline void construct(_T1* __p, const _T2& __value) {
  _Construct(__p, __value);
}

template <class _T1>
inline void construct(_T1* __p) {
  _Construct(__p);
}

template <class _Tp>
inline void destroy(_Tp* __pointer) {
  _Destroy(__pointer);
}

template <class _ForwardIterator>
inline void destroy(_ForwardIterator __first, _ForwardIterator __last) {
  _Destroy(__first, __last);
}

__STL_END_NAMESPACE

#endif /* __SGI_STL_INTERNAL_CONSTRUCT_H */

// Local Variables:
// mode:C++
// End:
