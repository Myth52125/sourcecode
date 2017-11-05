/*
 *
 * Copyright (c) 1997
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

#ifndef __TYPE_TRAITS_H
#define __TYPE_TRAITS_H

#ifndef __STL_CONFIG_H
#include <stl_config.h>
#endif

/*
This header file provides a framework for allowing compile time dispatch
based on type attributes. This is useful when writing template code.
For example, when making a copy of an array of an unknown type, it helps
to know if the type has a trivial copy constructor or not, to help decide
if a memcpy can be used.

The class template __type_traits provides a series of typedefs each of
which is either __true_type or __false_type. The argument to
__type_traits can be any type. The typedefs within this template will
attain their correct values by one of these means:
    1. The general instantiation contain conservative values which work
       for all types.
    2. Specializations may be declared to make distinctions between types.
    3. Some compilers (such as the Silicon Graphics N32 and N64 compilers)
       will automatically provide the appropriate specializations for all
       types.

EXAMPLE:

//Copy an array of elements which have non-trivial copy constructors
template <class T> void copy(T* source, T* destination, int n, __false_type);
//Copy an array of elements which have trivial copy constructors. Use memcpy.
template <class T> void copy(T* source, T* destination, int n, __true_type);

//Copy an array of any type by using the most efficient copy mechanism
template <class T> inline void copy(T* source,T* destination,int n) {
   copy(source, destination, n,
        typename __type_traits<T>::has_trivial_copy_constructor());
}
*/

// 下面这两个是标记符号，作为函数的参数。
// 能够返回一下两个个类型
// 这样编译器在根据函数参数类型的不同选择函数
// 实现了不同类型,不同对待

// 标记符号
struct __true_type
{
};
// 标记符号
struct __false_type
{
};

// 这里面的__true_type,__false_type表示的是
// 是否可以使用memcpy进行移动.__false_type表示必须使用构造函数.
// 如果一个类中包含指针，且动态分配了。
// 那也就是说,析构时候不能简单释放,要深拷贝,因此需要时有意义的,也就是__false_type
template <class _Tp>
struct __type_traits
{
  typedef __true_type this_dummy_member_must_be_first;
  // 这个是避免编译器也有同样的类型。但是。。。但是
  // 嗯不懂
  // 就是说有一些编译器可能就实现了这个玩意
  // 为了避免没冲突,加上这个,使得这个类型是编译器__type_traits的一个特例化
  /* Do not remove this member. It informs a compiler which
                      automatically specializes __type_traits that this
                      __type_traits template is special. It just makes sure that
                      things work if an implementation is using a template
                      called __type_traits for something unrelated. */

  /* The following restrictions should be observed for the sake of
      compilers which automatically produce type specific specializations 
      of this class:
          - You may reorder the members below if you wish
          - You may remove any of the members below if you wish
          - You must not rename members without making the corresponding
            name change in the compiler
          - Members you add will be treated like regular members unless
            you add the appropriate support in the compiler. */

  // 该结构体重的类型别名
  // 可以被其他的类型访问到
  // 嗯。。还缺点什么
  // 处于保守，表示类型的这些构造函数全都是有意义的。
  
  // 所以使用is_POD_type() 调用默认构造函数。

  typedef __false_type has_trivial_default_constructor;
  typedef __false_type has_trivial_copy_constructor;
  typedef __false_type has_trivial_assignment_operator;
  typedef __false_type has_trivial_destructor;
  typedef __false_type is_POD_type;
};

// Provide some specializations.  This is harmless for compilers that
//  have built-in __types_traits support, and essential for compilers
//  that don't.


// 下面是特化版本了.
// 这样
/*

    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    就能根据末班选择对用的true或是false

    而调用的函数，则有两个版本接受__true_type和接受__false_type的
*/
#ifndef __STL_NO_BOOL

__STL_TEMPLATE_NULL struct __type_traits<bool>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

#endif /* __STL_NO_BOOL */

__STL_TEMPLATE_NULL struct __type_traits<char>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

__STL_TEMPLATE_NULL struct __type_traits<signed char>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

__STL_TEMPLATE_NULL struct __type_traits<unsigned char>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

#ifdef __STL_HAS_WCHAR_T

__STL_TEMPLATE_NULL struct __type_traits<wchar_t>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

#endif /* __STL_HAS_WCHAR_T */

__STL_TEMPLATE_NULL struct __type_traits<short>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

__STL_TEMPLATE_NULL struct __type_traits<unsigned short>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

__STL_TEMPLATE_NULL struct __type_traits<int>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

__STL_TEMPLATE_NULL struct __type_traits<unsigned int>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

__STL_TEMPLATE_NULL struct __type_traits<long>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

__STL_TEMPLATE_NULL struct __type_traits<unsigned long>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

#ifdef __STL_LONG_LONG

__STL_TEMPLATE_NULL struct __type_traits<long long>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

__STL_TEMPLATE_NULL struct __type_traits<unsigned long long>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

#endif /* __STL_LONG_LONG */

__STL_TEMPLATE_NULL struct __type_traits<float>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

__STL_TEMPLATE_NULL struct __type_traits<double>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

__STL_TEMPLATE_NULL struct __type_traits<long double>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

#ifdef __STL_CLASS_PARTIAL_SPECIALIZATION

// 指针类型
// 原生的指针.指针传参本来就是值传递嘛.
template <class _Tp>
struct __type_traits<_Tp *>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

#else /* __STL_CLASS_PARTIAL_SPECIALIZATION */

__STL_TEMPLATE_NULL struct __type_traits<char *>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

__STL_TEMPLATE_NULL struct __type_traits<signed char *>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

__STL_TEMPLATE_NULL struct __type_traits<unsigned char *>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

__STL_TEMPLATE_NULL struct __type_traits<const char *>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

__STL_TEMPLATE_NULL struct __type_traits<const signed char *>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

__STL_TEMPLATE_NULL struct __type_traits<const unsigned char *>
{
  typedef __true_type has_trivial_default_constructor;
  typedef __true_type has_trivial_copy_constructor;
  typedef __true_type has_trivial_assignment_operator;
  typedef __true_type has_trivial_destructor;
  typedef __true_type is_POD_type;
};

#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */

// The following could be written in terms of numeric_limits.
// We're doing it separately to reduce the number of dependencies.

// 是不是整数
template <class _Tp>
struct _Is_integer
{
  // 如果没有特化版本那么返回的就是__false_type.从而选择的是其他的函数
  typedef __false_type _Integral;
};

#ifndef __STL_NO_BOOL

__STL_TEMPLATE_NULL struct _Is_integer<bool>
{
  typedef __true_type _Integral;
};

#endif /* __STL_NO_BOOL */

// 特化版本
__STL_TEMPLATE_NULL struct _Is_integer<char>
{
  typedef __true_type _Integral;
};

__STL_TEMPLATE_NULL struct _Is_integer<signed char>
{
  typedef __true_type _Integral;
};

__STL_TEMPLATE_NULL struct _Is_integer<unsigned char>
{
  typedef __true_type _Integral;
};

#ifdef __STL_HAS_WCHAR_T

__STL_TEMPLATE_NULL struct _Is_integer<wchar_t>
{
  typedef __true_type _Integral;
};

#endif /* __STL_HAS_WCHAR_T */

__STL_TEMPLATE_NULL struct _Is_integer<short>
{
  typedef __true_type _Integral;
};

__STL_TEMPLATE_NULL struct _Is_integer<unsigned short>
{
  typedef __true_type _Integral;
};

__STL_TEMPLATE_NULL struct _Is_integer<int>
{
  typedef __true_type _Integral;
};

__STL_TEMPLATE_NULL struct _Is_integer<unsigned int>
{
  typedef __true_type _Integral;
};

__STL_TEMPLATE_NULL struct _Is_integer<long>
{
  typedef __true_type _Integral;
};

__STL_TEMPLATE_NULL struct _Is_integer<unsigned long>
{
  typedef __true_type _Integral;
};

#ifdef __STL_LONG_LONG

__STL_TEMPLATE_NULL struct _Is_integer<long long>
{
  typedef __true_type _Integral;
};

__STL_TEMPLATE_NULL struct _Is_integer<unsigned long long>
{
  typedef __true_type _Integral;
};

#endif /* __STL_LONG_LONG */

#endif /* __TYPE_TRAITS_H */

// Local Variables:
// mode:C++
// End:
