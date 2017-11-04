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
 * Copyright (c) 1996
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

#ifndef __SGI_STL_INTERNAL_VECTOR_H
#define __SGI_STL_INTERNAL_VECTOR_H

#include <concept_checks.h>

__STL_BEGIN_NAMESPACE

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#pragma set woff 1375
#endif

// The vector base class serves two purposes.  First, its constructor
// and destructor allocate (but don't initialize) storage.  This makes
// exception safety easier.  Second, the base class encapsulates all of
// the differences between SGI-style allocators and standard-conforming
// allocators.

#ifdef __STL_USE_STD_ALLOCATORS

// Base class for ordinary allocators.
//一个为vector分配内存的基类。
//传入的内存分配器，应该是二级分配器
template <class _Tp, class _Allocator, bool _IsStatic>
class _Vector_alloc_base
{
public:
  typedef typename _Alloc_traits<_Tp, _Allocator>::allocator_type
      allocator_type;
  allocator_type get_allocator() const { return _M_data_allocator; }

  _Vector_alloc_base(const allocator_type &__a)
      : _M_data_allocator(__a), _M_start(0), _M_finish(0), _M_end_of_storage(0)
  {
  }

protected:
  //分配器
  allocator_type _M_data_allocator;
  _Tp *_M_start;
  //已使用空间的尾地址
  _Tp *_M_finish;
  //分配空间的尾地址
  _Tp *_M_end_of_storage;

  //封装了分配器的分配函数
  //没有重新分配空间的函数
  _Tp *_M_allocate(size_t __n)
  {
    return _M_data_allocator.allocate(__n);
  }
  //释放内存的函数
  void _M_deallocate(_Tp *__p, size_t __n)
  {
    if (__p)
      _M_data_allocator.deallocate(__p, __n);
  }
};

// Specialization for allocators that have the property that we don't
// actually have to store an allocator object.
//特化版本，没有存储二级分配器。
//应该用的是一级分配器吧。好像还不是
//_Alloc_traits<_Tp, _Allocator>是个啥？
template <class _Tp, class _Allocator>
class _Vector_alloc_base<_Tp, _Allocator, true>
{
public:
  typedef typename _Alloc_traits<_Tp, _Allocator>::allocator_type
      allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }

  _Vector_alloc_base(const allocator_type &)
      : _M_start(0), _M_finish(0), _M_end_of_storage(0)
  {
  }

protected:
  _Tp *_M_start;
  _Tp *_M_finish;
  _Tp *_M_end_of_storage;

  typedef typename _Alloc_traits<_Tp, _Allocator>::_Alloc_type _Alloc_type;
  _Tp *_M_allocate(size_t __n)
  {
    return _Alloc_type::allocate(__n);
  }
  void _M_deallocate(_Tp *__p, size_t __n)
  {
    _Alloc_type::deallocate(__p, __n);
  }
};

//上一层的简单封装，但是封装了个啥？
template <class _Tp, class _Alloc>
struct _Vector_base
    : public _Vector_alloc_base<_Tp, _Alloc,
                                _Alloc_traits<_Tp, _Alloc>::_S_instanceless>
{
  typedef _Vector_alloc_base<_Tp, _Alloc,
                             _Alloc_traits<_Tp, _Alloc>::_S_instanceless>
      _Base;
  typedef typename _Base::allocator_type allocator_type;

  _Vector_base(const allocator_type &__a) : _Base(__a) {}
  _Vector_base(size_t __n, const allocator_type &__a) : _Base(__a)
  {
    _M_start = _M_allocate(__n);
    _M_finish = _M_start;
    _M_end_of_storage = _M_start + __n;
  }
  ~_Vector_base() { _M_deallocate(_M_start, _M_end_of_storage - _M_start); }
};

#else /* __STL_USE_STD_ALLOCATORS */

//没有存储内存分配器的版本
template <class _Tp, class _Alloc>
class _Vector_base
{
public:
  typedef _Alloc allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }

  _Vector_base(const _Alloc &)
      : _M_start(0), _M_finish(0), _M_end_of_storage(0) {}
  _Vector_base(size_t __n, const _Alloc &)
      : _M_start(0), _M_finish(0), _M_end_of_storage(0)
  {
    _M_start = _M_allocate(__n);
    _M_finish = _M_start;
    _M_end_of_storage = _M_start + __n;
  }
  // 这个析构函数不是一个虚函数啊!!!
  // 因为根本不会用到多态!
  // 所以不需要是虚函数
  ~_Vector_base() { _M_deallocate(_M_start, _M_end_of_storage - _M_start); }

protected:
  _Tp *_M_start;
  _Tp *_M_finish;
  _Tp *_M_end_of_storage;

  typedef simple_alloc<_Tp, _Alloc> _M_data_allocator;
  _Tp *_M_allocate(size_t __n)
  {
    return _M_data_allocator::allocate(__n);
  }
  void _M_deallocate(_Tp *__p, size_t __n)
  {
    _M_data_allocator::deallocate(__p, __n);
  }
};

#endif /* __STL_USE_STD_ALLOCATORS */

//vector部分，貌似可以再继承。
template <class _Tp, class _Alloc = __STL_DEFAULT_ALLOCATOR(_Tp)>
class vector : protected _Vector_base<_Tp, _Alloc>
{
  // requirements:

  __STL_CLASS_REQUIRES(_Tp, _Assignable);
  /*
#define __STL_CLASS_REQUIRES(__type_var, __concept) \
  static int  __##__type_var##_##__concept
  上面结果为static int _Tp__Assignable,定义了一个变量。
*/
private:
  typedef _Vector_base<_Tp, _Alloc> _Base;

public:
  // 首先是各种数据类型
  typedef _Tp value_type;
  typedef value_type *pointer;
  typedef const value_type *const_pointer;
  typedef value_type *iterator;
  typedef const value_type *const_iterator;
  typedef value_type &reference;
  typedef const value_type &const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  typedef typename _Base::allocator_type allocator_type;
  allocator_type get_allocator() const { return _Base::get_allocator(); }

  // 迭代器
#ifdef __STL_CLASS_PARTIAL_SPECIALIZATION
  typedef reverse_iterator<const_iterator> const_reverse_iterator;
  typedef reverse_iterator<iterator> reverse_iterator;
#else  /* __STL_CLASS_PARTIAL_SPECIALIZATION */
  typedef reverse_iterator<const_iterator, value_type, const_reference,
                           difference_type>
      const_reverse_iterator;
  typedef reverse_iterator<iterator, value_type, reference, difference_type>
      reverse_iterator;
#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */

protected:
#ifdef __STL_HAS_NAMESPACES
  using _Base::_M_allocate;
  using _Base::_M_deallocate;
  using _Base::_M_end_of_storage;
  using _Base::_M_finish;
  using _Base::_M_start;
#endif /* __STL_HAS_NAMESPACES */

protected:
  void _M_insert_aux(iterator __position, const _Tp &__x);
  void _M_insert_aux(iterator __position);

public:
  // 返回迭代器的函数
  // 这里直接返回了指针
  iterator begin() { return _M_start; }
  const_iterator begin() const { return _M_start; }
  iterator end() { return _M_finish; }
  const_iterator end() const { return _M_finish; }

  // 反向迭代器
  reverse_iterator rbegin()
  {
    return reverse_iterator(end());
  }
  const_reverse_iterator rbegin() const
  {
    return const_reverse_iterator(end());
  }
  reverse_iterator rend()
  {
    return reverse_iterator(begin());
  }
  const_reverse_iterator rend() const
  {
    return const_reverse_iterator(begin());
  }

  // 这里又一次转型
  size_type size() const
  {
    return size_type(end() - begin());
  }
  size_type max_size() const
  {
    return size_type(-1) / sizeof(_Tp);
  }
  // 这个返回的是还能存储多少
  size_type capacity() const
  {
    return size_type(_M_end_of_storage - begin());
  }
  // 头等于尾，是空
  bool empty() const
  {
    return begin() == end();
  }
  // 下标，使用的是偏移
  // 证明了这个运算符不会检查越界
  reference operator[](size_type __n) { return *(begin() + __n); }
  const_reference operator[](size_type __n) const { return *(begin() + __n); }

#ifdef __STL_THROW_RANGE_ERRORS
  // 越界检查函数，给at调用
  void _M_range_check(size_type __n) const
  {
    if (__n >= this->size())
      __stl_throw_range_error("vector");
  }
  // 带越界检查
  reference at(size_type __n)
  {
    _M_range_check(__n);
    return (*this)[__n];
  }
  const_reference at(size_type __n) const
  {
    _M_range_check(__n);
    return (*this)[__n];
  }
#endif /* __STL_THROW_RANGE_ERRORS */

  // 算是默认的构造函数,直接接受一个分配器,存储在内存
  explicit vector(const allocator_type &__a = allocator_type())
      : _Base(__a)
  {
  }
  // 第一个参数是个数,第二个参数是值
  vector(size_type __n, const _Tp &__value,
         const allocator_type &__a = allocator_type())
      : _Base(__n, __a)
  { 
  // 这个函数还没看。应该是分配空间，并且设置值的吧。
    _M_finish = uninitialized_fill_n(_M_start, __n, __value);
  }

  explicit vector(size_type __n)
      : _Base(__n, allocator_type())
  {
    _M_finish = uninitialized_fill_n(_M_start, __n, _Tp());
  }
  // 宝贝构造函数
  // 拷贝了内存分配器，大小，然后还复制了内容
  vector(const vector<_Tp, _Alloc> &__x)
      : _Base(__x.size(), __x.get_allocator())
  {
    _M_finish = uninitialized_copy(__x.begin(), __x.end(), _M_start);
  }

#ifdef __STL_MEMBER_TEMPLATES
  // Check whether it's an integral type.  If so, it's not an iterator.
  template <class _InputIterator>
  vector(_InputIterator __first, _InputIterator __last,
         const allocator_type &__a = allocator_type()) : _Base(__a)
  {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    // 判断是否是整数？但是怎么判断出来的？
    _M_initialize_aux(__first, __last, _Integral());
  }

  // __true_type就是个标记符号，用来区别两个模板函数的

  // 这个是，当前两个值时整数的时候，第一个是个数，第二个就是值
  template <class _Integer>
  void _M_initialize_aux(_Integer __n, _Integer __value, __true_type)
  {
    _M_start = _M_allocate(__n);
    _M_end_of_storage = _M_start + __n;
    _M_finish = uninitialized_fill_n(_M_start, __n, __value);
  }
  // 不是整数的时候，表示两个是迭代器
  template <class _InputIterator>
  void _M_initialize_aux(_InputIterator __first, _InputIterator __last,
                         __false_type)
  {
    _M_range_initialize(__first, __last, __ITERATOR_CATEGORY(__first));
  }

#else
  vector(const _Tp *__first, const _Tp *__last,
         const allocator_type &__a = allocator_type())
      : _Base(__last - __first, __a)
  {
    _M_finish = uninitialized_copy(__first, __last, _M_start);
  }
#endif /* __STL_MEMBER_TEMPLATES */

  // 析构函数
  // 内存回收?
  // 资源回收,是在基类的使用的二级内存分配器中回收.accept
  // 也就是说vector如果不是析构,那么所占内存不会被回收.回收也是回收给二级分配器,
  // 如果分配的空间很大,那么是直接释放的.
  ~vector()
  {
    destroy(_M_start, _M_finish);
  }

  // 重新分配大小的函数
  vector<_Tp, _Alloc> &operator=(const vector<_Tp, _Alloc> &__x);
  void reserve(size_type __n)
  {
    // 判断申请的空间够不够
    if (capacity() < __n)
    {
      const size_type __old_size = size();
      // 这里应该是重新分配了空间，并且拷贝了
      iterator __tmp = _M_allocate_and_copy(__n, _M_start, _M_finish);
      // 这个析构函数，是执行元素的释放，不是释放空间
      destroy(_M_start, _M_finish);
      // 这个析构函数，释放内存空间的函数
      // 释放空间的时候都会调用这个函数
      _M_deallocate(_M_start, _M_end_of_storage - _M_start);
      // 重新设置指针。
      // 所以之前的指针会失效
      _M_start = __tmp;
      _M_finish = __tmp + __old_size;
      _M_end_of_storage = _M_start + __n;
    }
  }

  // assign(), a generalized assignment member function.  Two
  // versions: one that takes a count, and one that takes a range.
  // The range version is a member template, so we dispatch on whether
  // or not the type is an integer.

  // 分配？
  void assign(size_type __n, const _Tp &__val) { _M_fill_assign(__n, __val); }
  void _M_fill_assign(size_type __n, const _Tp &__val);

#ifdef __STL_MEMBER_TEMPLATES
  // 这个和上面一样的套路，需要判断是否是迭代器还是整数
  template <class _InputIterator>
  void assign(_InputIterator __first, _InputIterator __last)
  {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_assign_dispatch(__first, __last, _Integral());
  }

  // 两个别调用的函数
  template <class _Integer>
  void _M_assign_dispatch(_Integer __n, _Integer __val, __true_type)
  {
    _M_fill_assign((size_type)__n, (_Tp)__val);
  }

  template <class _InputIter>
  void _M_assign_dispatch(_InputIter __first, _InputIter __last, __false_type)
  {
    _M_assign_aux(__first, __last, __ITERATOR_CATEGORY(__first));
  }

  template <class _InputIterator>
  void _M_assign_aux(_InputIterator __first, _InputIterator __last,
                     input_iterator_tag);

  template <class _ForwardIterator>
  void _M_assign_aux(_ForwardIterator __first, _ForwardIterator __last,
                     forward_iterator_tag);

#endif /* __STL_MEMBER_TEMPLATES */
  // 为什么不反悔第一个元素？还要在构建以此迭代器？
  reference front()
  {
    return *begin();
  }
  const_reference front() const { return *begin(); }
  // 这里有个-1
  reference back() { return *(end() - 1); }
  const_reference back() const { return *(end() - 1); }

  // 重头戏，主要的函数

  // 先判断是不是到了结尾
  // 因为申请内存的时候是按照n*size，所以两个指针会相遇
  // push什么都不返回啊
  void push_back(const _Tp &__x)
  {
    if (_M_finish != _M_end_of_storage)
    {
      // 如果没相遇，那么要添加。使用了这么个函数？
      construct(_M_finish, __x);
      ++_M_finish;
    }
    else
    // 这个应该是要重新分配内存了
      _M_insert_aux(end(), __x);
  }
  // 空的？？？干了什么？
  void push_back()
  {
    if (_M_finish != _M_end_of_storage)
    {
      construct(_M_finish);
      ++_M_finish;
    }
    else
      _M_insert_aux(end());
  }
  // 交换，哈？直接使用了__std的
  // 交换了，内存的三个指针。
  void swap(vector<_Tp, _Alloc> &__x)
  {
    __STD::swap(_M_start, __x._M_start);
    __STD::swap(_M_finish, __x._M_finish);
    __STD::swap(_M_end_of_storage, __x._M_end_of_storage);
  }

  iterator insert(iterator __position, const _Tp &__x)
  {
    // 插入的位置。
    size_type __n = __position - begin();
    // 是不是在结尾
    if (_M_finish != _M_end_of_storage && __position == end())
    {
      construct(_M_finish, __x);
      ++_M_finish;
    }
    else
    // 不是在结尾。这个函数挺重要的啊
      _M_insert_aux(__position, __x);
    // 返回插入的元素
    // 为什么要返回插入的元素？
    return begin() + __n;
  }
  // 又是个空的?是插入默认的？
  iterator insert(iterator __position)
  {
    size_type __n = __position - begin();
    if (_M_finish != _M_end_of_storage && __position == end())
    {
      construct(_M_finish);
      ++_M_finish;
    }
    else
      _M_insert_aux(__position);
    return begin() + __n;
  }
#ifdef __STL_MEMBER_TEMPLATES
  // Check whether it's an integral type.  If so, it's not an iterator.
  // 插入另一个容器的一个范围内的元素
  template <class _InputIterator>
  void insert(iterator __pos, _InputIterator __first, _InputIterator __last)
  {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_insert_dispatch(__pos, __first, __last, _Integral());
  }
  // 同样的套路，需要判断是否是整数，也就是从每个迭代器开始，插入n个整数

  // 整数的时候，直接填充
  template <class _Integer>
  void _M_insert_dispatch(iterator __pos, _Integer __n, _Integer __val,
                          __true_type)
  {
    _M_fill_insert(__pos, (size_type)__n, (_Tp)__val);
  }

  // 类类型的时候进行范围插入
  template <class _InputIterator>
  void _M_insert_dispatch(iterator __pos,
                          _InputIterator __first, _InputIterator __last,
                          __false_type)
  {
    _M_range_insert(__pos, __first, __last, __ITERATOR_CATEGORY(__first));
  }
#else  /* __STL_MEMBER_TEMPLATES */
  void insert(iterator __position,
              const_iterator __first, const_iterator __last);
#endif /* __STL_MEMBER_TEMPLATES */

  void insert(iterator __pos, size_type __n, const _Tp &__x)
  {
    _M_fill_insert(__pos, __n, __x);
  }

  void _M_fill_insert(iterator __pos, size_type __n, const _Tp &__x);

  // 
  void pop_back()
  {
    --_M_finish;
    // 这个函数，将元素析构掉。
    destroy(_M_finish);
  }

  iterator erase(iterator __position)
  {
    // 如果要删除的对象不是最后一个，那么需要移动后面的
    if (__position + 1 != end())
      copy(__position + 1, _M_finish, __position);
    // 需要进行析构
    --_M_finish;
    destroy(_M_finish);
    return __position;
  }
  // 擦出一个范围
  iterator erase(iterator __first, iterator __last)
  {
    // copy返回的是什么？
    iterator __i = copy(__last, _M_finish, __first);
    destroy(__i, _M_finish);
    _M_finish = _M_finish - (__last - __first);
    return __first;
  }

  void resize(size_type __new_size, const _Tp &__x)
  {
    // 如果是缩小，那么直接查处后面的
    if (__new_size < size())
      erase(begin() + __new_size, end());
    else
    // 是放大，并且放大后空间的的值有要求
      insert(end(), __new_size - size(), __x);
  }
  void resize(size_type __new_size) { resize(__new_size, _Tp()); }
  void clear() { erase(begin(), end()); }

protected:
#ifdef __STL_MEMBER_TEMPLATES
  template <class _ForwardIterator>
  // 分配并复制
  iterator _M_allocate_and_copy(size_type __n, _ForwardIterator __first,
                                _ForwardIterator __last)
  {
    // 分配新的空间
    iterator __result = _M_allocate(__n);
    // 这就是个try
    __STL_TRY
    {
      // 复制过去
      uninitialized_copy(__first, __last, __result);
      return __result;
    }
    // catch，如果失败，就释放空间
    __STL_UNWIND(_M_deallocate(__result, __n));
  }
#else  /* __STL_MEMBER_TEMPLATES */
  iterator _M_allocate_and_copy(size_type __n, const_iterator __first,
                                const_iterator __last)
  {
    iterator __result = _M_allocate(__n);
    __STL_TRY
    {
      uninitialized_copy(__first, __last, __result);
      return __result;
    }
    __STL_UNWIND(_M_deallocate(__result, __n));
  }
#endif /* __STL_MEMBER_TEMPLATES */

#ifdef __STL_MEMBER_TEMPLATES
  // 我擦，这个范围插入，就是个for+push啊，
  // 哦，如果是最后一个的话input_iterator_tag是个标记
  template <class _InputIterator>
  void _M_range_initialize(_InputIterator __first,
                           _InputIterator __last, input_iterator_tag)
  {
    for (; __first != __last; ++__first)
      push_back(*__first);
  }

  // This function is only called by the constructor.
  template <class _ForwardIterator>
  void _M_range_initialize(_ForwardIterator __first,
                           _ForwardIterator __last, forward_iterator_tag)
  {
    // 没看懂。
    size_type __n = 0;
    distance(__first, __last, __n);
    _M_start = _M_allocate(__n);
    _M_end_of_storage = _M_start + __n;
    _M_finish = uninitialized_copy(__first, __last, _M_start);
  }

  template <class _InputIterator>
  void _M_range_insert(iterator __pos,
                       _InputIterator __first, _InputIterator __last,
                       input_iterator_tag);

  template <class _ForwardIterator>
  void _M_range_insert(iterator __pos,
                       _ForwardIterator __first, _ForwardIterator __last,
                       forward_iterator_tag);

#endif /* __STL_MEMBER_TEMPLATES */
};


// 运算符，只定义了两个，等于，和大于，其他的都可以在这上面转化
// 相等运算符
template <class _Tp, class _Alloc>
inline bool
operator==(const vector<_Tp, _Alloc> &__x, const vector<_Tp, _Alloc> &__y)
{
  // 首先判断是否大小相同，然后应该是挨个判断了吧。
  return __x.size() == __y.size() &&
         equal(__x.begin(), __x.end(), __y.begin());
}

template <class _Tp, class _Alloc>
inline bool
operator<(const vector<_Tp, _Alloc> &__x, const vector<_Tp, _Alloc> &__y)
{
  return lexicographical_compare(__x.begin(), __x.end(),
                                 __y.begin(), __y.end());
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER

// swap(),就是调用内部的swap
template <class _Tp, class _Alloc>
inline void swap(vector<_Tp, _Alloc> &__x, vector<_Tp, _Alloc> &__y)
{
  __x.swap(__y);
}

template <class _Tp, class _Alloc>
inline bool
operator!=(const vector<_Tp, _Alloc> &__x, const vector<_Tp, _Alloc> &__y)
{
  return !(__x == __y);
}

// 23333
template <class _Tp, class _Alloc>
inline bool
operator>(const vector<_Tp, _Alloc> &__x, const vector<_Tp, _Alloc> &__y)
{
  return __y < __x;
}

// 23333
template <class _Tp, class _Alloc>
inline bool
operator<=(const vector<_Tp, _Alloc> &__x, const vector<_Tp, _Alloc> &__y)
{
  return !(__y < __x);
}

template <class _Tp, class _Alloc>
inline bool
operator>=(const vector<_Tp, _Alloc> &__x, const vector<_Tp, _Alloc> &__y)
{
  return !(__x < __y);
}

#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */

// 赋值运算符
template <class _Tp, class _Alloc>
vector<_Tp, _Alloc> &
vector<_Tp, _Alloc>::operator=(const vector<_Tp, _Alloc> &__x)
{
  // 使用了引用类型，然后先判断是否相同
  if (&__x != this)
  {
    // 需要有足够的空间。
    const size_type __xlen = __x.size();
    // 空间不够
    if (__xlen > capacity())
    {
      // 复制进去
      iterator __tmp = _M_allocate_and_copy(__xlen, __x.begin(), __x.end());
      // 然后析构掉之前的那
      // 但是，我曹，这个内存地址改变了啊
      destroy(_M_start, _M_finish);
      _M_deallocate(_M_start, _M_end_of_storage - _M_start);
      // 改变了元素的地址。
      _M_start = __tmp;
      _M_end_of_storage = _M_start + __xlen;
    }
    else if (size() >= __xlen)
    {
      // 复制进来的比已使用的小
      iterator __i = copy(__x.begin(), __x.end(), begin());
      // 然后析构掉所有的。
      destroy(__i, _M_finish);
    }
    else
    {
      // 也就是要拷贝进来的比现有的元素数量多，但是比总容量少
      copy(__x.begin(), __x.begin() + size(), _M_start);
      // 直接拷贝进去，然后析构之前的。
      uninitialized_copy(__x.begin() + size(), __x.end(), _M_finish);
    }
    // 改变他的那个已用空间
    _M_finish = _M_start + __xlen;
  }
  // 赋值运算符返回自身引用.
  return *this;
}

template <class _Tp, class _Alloc>
void vector<_Tp, _Alloc>::_M_fill_assign(size_t __n, const value_type &__val)
{
  if (__n > capacity())
  {
    // 如果自身不满足要求分配的大小.
    // 建立一个新的vector，然后交换，再析构
    vector<_Tp, _Alloc> __tmp(__n, __val, get_allocator());
    __tmp.swap(*this);
  }
  else if (__n > size())
  {
    // 如果n大于已分配的数量，比总容量小
    // 直接填充,把已有的改变了
    fill(begin(), end(), __val);
    // 把后边的填充了,但是之前原有的不去析构了?fill会析构啊
    _M_finish = uninitialized_fill_n(_M_finish, __n - size(), __val);
  }
  else
  // 也就是,小于size(),那么直接填充
    erase(fill_n(begin(), __n, __val), end());
}

#ifdef __STL_MEMBER_TEMPLATES

template <class _Tp, class _Alloc>
template <class _InputIter>
void vector<_Tp, _Alloc>::_M_assign_aux(_InputIter __first, _InputIter __last,
                                        input_iterator_tag)
{
  iterator __cur = begin();
  for (; __first != __last && __cur != end(); ++__cur, ++__first)
    *__cur = *__first;
  if (__first == __last)
    erase(__cur, end());
  else
    insert(end(), __first, __last);
}

template <class _Tp, class _Alloc>
template <class _ForwardIter>
void vector<_Tp, _Alloc>::_M_assign_aux(_ForwardIter __first, _ForwardIter __last,
                                        forward_iterator_tag)
{
  size_type __len = 0;
  // 这个函数是个啥?,传出距离?嗯,传出两个指针的距离
  distance(__first, __last, __len);
  // 如果空间小,那就重新申请一块,然后释放以前的.
  if (__len > capacity())
  {
    iterator __tmp = _M_allocate_and_copy(__len, __first, __last);
    destroy(_M_start, _M_finish);
    _M_deallocate(_M_start, _M_end_of_storage - _M_start);
    _M_start = __tmp;
    _M_end_of_storage = _M_finish = _M_start + __len;
  }
  else if (size() >= __len)
  {
    // 小于已有的数量.
    iterator __new_finish = copy(__first, __last, _M_start);
    // 析构已有的
    destroy(__new_finish, _M_finish);
    _M_finish = __new_finish;
  }
  else
  {
    // 在size和容量之间,
    _ForwardIter __mid = __first;
    // 向前移动size?
    advance(__mid, size());
    // 哦哦,分两部分拷贝,copy也有析构的功能啊
    copy(__first, __mid, _M_start);
    // 在拷贝后面的部分
    _M_finish = uninitialized_copy(__mid, __last, _M_finish);
  }
  // 这个没有改变头指针,
}

#endif /* __STL_MEMBER_TEMPLATES */

template <class _Tp, class _Alloc>
void vector<_Tp, _Alloc>::_M_insert_aux(iterator __position, const _Tp &__x)
{
  // 空间满了
  if (_M_finish != _M_end_of_storage)
  {
    // 在结尾处构造一个
    construct(_M_finish, *(_M_finish - 1));
    // 元素尾指针后移
    // **************没看懂
    ++_M_finish;
    _Tp __x_copy = __x;
    // 哦哦,这是个向后移动的函数啊.所以要县构造了一次
    // 没有析构,只是移动
    copy_backward(__position, _M_finish - 2, _M_finish - 1);
    *__position = __x_copy;
  }
  else
  {
    // 默认初始化,空间为0;
    const size_type __old_size = size();
    // 满了就分配两倍的空间
    // 如果初始是后为0,就分配一个.
    const size_type __len = __old_size != 0 ? 2 * __old_size : 1;
    iterator __new_start = _M_allocate(__len);
    iterator __new_finish = __new_start;
    __STL_TRY
    {
      // 拷贝到新的位置
      __new_finish = uninitialized_copy(_M_start, __position, __new_start);
      // 在尾部插入一个新的元素
      construct(__new_finish, __x);
      // 移动尾部
      ++__new_finish;
      // 然后将后面的拷贝进来.因为是中间插入
      __new_finish = uninitialized_copy(__position, _M_finish, __new_finish);
    }
    __STL_UNWIND((destroy(__new_start, __new_finish),
                  _M_deallocate(__new_start, __len)));
    // 涉及到重新分配空间了,所以析构了之前的.
    // 先析构了元素,然后再释放了空间
    destroy(begin(), end());
    _M_deallocate(_M_start, _M_end_of_storage - _M_start);
    // 设置新的指针
    _M_start = __new_start;
    _M_finish = __new_finish;
    _M_end_of_storage = __new_start + __len;
  }
}

// 就是插入了一个使用默认构造函数构造的对象。
// 基本和上面一个套路了
template <class _Tp, class _Alloc>
void vector<_Tp, _Alloc>::_M_insert_aux(iterator __position)
{
  if (_M_finish != _M_end_of_storage)
  {
    // 在固定的位置调用拷贝构造函数。
    construct(_M_finish, *(_M_finish - 1));
    ++_M_finish;
    copy_backward(__position, _M_finish - 2, _M_finish - 1);
    *__position = _Tp();
  }
  else
  {
    const size_type __old_size = size();
    const size_type __len = __old_size != 0 ? 2 * __old_size : 1;
    iterator __new_start = _M_allocate(__len);
    iterator __new_finish = __new_start;
    __STL_TRY
    {
      __new_finish = uninitialized_copy(_M_start, __position, __new_start);
      construct(__new_finish);
      ++__new_finish;
      __new_finish = uninitialized_copy(__position, _M_finish, __new_finish);
    }
    __STL_UNWIND((destroy(__new_start, __new_finish),
                  _M_deallocate(__new_start, __len)));
    destroy(begin(), end());
    _M_deallocate(_M_start, _M_end_of_storage - _M_start);
    _M_start = __new_start;
    _M_finish = __new_finish;
    _M_end_of_storage = __new_start + __len;
  }
}


// 范围插入的辅助函数
// 第一个是插入n个相同的对象
template <class _Tp, class _Alloc>
void vector<_Tp, _Alloc>::_M_fill_insert(iterator __position, size_type __n,
                                         const _Tp &__x)
{
  // 需要插入的个数不为0
  if (__n != 0)
  {
    if (size_type(_M_end_of_storage - _M_finish) >= __n)
    {
      // 剩余空间足够插入
      // 构造出来
      _Tp __x_copy = __x;
      // 需要移动的对象个数
      const size_type __elems_after = _M_finish - __position;
      iterator __old_finish = _M_finish;
      // 需要移动的对象个数比要插入的多
      if (__elems_after > __n)
      {
        // 也是分两段拷贝。为什么不从开始直接拷贝到结束啊？
        uninitialized_copy(_M_finish - __n, _M_finish, _M_finish);
        _M_finish += __n;
        copy_backward(__position, __old_finish - __n, __old_finish);
        fill(__position, __position + __n, __x_copy);
      }
      else
      {
        // 嗯，懂！
        uninitialized_fill_n(_M_finish, __n - __elems_after, __x_copy);
        _M_finish += __n - __elems_after;
        uninitialized_copy(__position, __old_finish, _M_finish);
        _M_finish += __elems_after;
        fill(__position, __old_finish, __x_copy);
      }
    }
    else
    {
      // 剩余空间不够了，要重新分配空间。
      const size_type __old_size = size();
      const size_type __len = __old_size + max(__old_size, __n);
      iterator __new_start = _M_allocate(__len);
      iterator __new_finish = __new_start;
      __STL_TRY
      {
        __new_finish = uninitialized_copy(_M_start, __position, __new_start);
        __new_finish = uninitialized_fill_n(__new_finish, __n, __x);
        __new_finish = uninitialized_copy(__position, _M_finish, __new_finish);
      }
      __STL_UNWIND((destroy(__new_start, __new_finish),
                    _M_deallocate(__new_start, __len)));
      destroy(_M_start, _M_finish);
      _M_deallocate(_M_start, _M_end_of_storage - _M_start);
      _M_start = __new_start;
      _M_finish = __new_finish;
      _M_end_of_storage = __new_start + __len;
    }
  }
}

#ifdef __STL_MEMBER_TEMPLATES

// 插入类类型，就是一个个插入的？
// 不会吧？
// 效率不高啊,每次都要移动啊
// 哦,这个是在尾部插入,哈哈
// 但是也不想啊,尾部插入直接使用push啊
template <class _Tp, class _Alloc>
template <class _InputIterator>
void vector<_Tp, _Alloc>::_M_range_insert(iterator __pos,
                                          _InputIterator __first,
                                          _InputIterator __last,
                                          input_iterator_tag)
{
  for (; __first != __last; ++__first)
  {
    __pos = insert(__pos, *__first);
    ++__pos;
  }
}

template <class _Tp, class _Alloc>
template <class _ForwardIterator>
void vector<_Tp, _Alloc>::_M_range_insert(iterator __position,
                                          _ForwardIterator __first,
                                          _ForwardIterator __last,
                                          forward_iterator_tag)
{
  if (__first != __last)
  {
    size_type __n = 0;
    // 获取需要插入的个数
    distance(__first, __last, __n);
    if (size_type(_M_end_of_storage - _M_finish) >= __n)
    {
      // 剩余空间足够插入
      // 和上面的套路一样了
      const size_type __elems_after = _M_finish - __position;
      iterator __old_finish = _M_finish;
      if (__elems_after > __n)
      {
        uninitialized_copy(_M_finish - __n, _M_finish, _M_finish);
        _M_finish += __n;
        copy_backward(__position, __old_finish - __n, __old_finish);
        copy(__first, __last, __position);
      }
      else
      {
        _ForwardIterator __mid = __first;
        advance(__mid, __elems_after);
        uninitialized_copy(__mid, __last, _M_finish);
        _M_finish += __n - __elems_after;
        uninitialized_copy(__position, __old_finish, _M_finish);
        _M_finish += __elems_after;
        copy(__first, __mid, __position);
      }
    }
    else
    {
      const size_type __old_size = size();
      const size_type __len = __old_size + max(__old_size, __n);
      iterator __new_start = _M_allocate(__len);
      iterator __new_finish = __new_start;
      __STL_TRY
      {
        __new_finish = uninitialized_copy(_M_start, __position, __new_start);
        __new_finish = uninitialized_copy(__first, __last, __new_finish);
        __new_finish = uninitialized_copy(__position, _M_finish, __new_finish);
      }
      __STL_UNWIND((destroy(__new_start, __new_finish),
                    _M_deallocate(__new_start, __len)));
      destroy(_M_start, _M_finish);
      _M_deallocate(_M_start, _M_end_of_storage - _M_start);
      _M_start = __new_start;
      _M_finish = __new_finish;
      _M_end_of_storage = __new_start + __len;
    }
  }
}

#else /* __STL_MEMBER_TEMPLATES */

// 套路都一样
template <class _Tp, class _Alloc>
void vector<_Tp, _Alloc>::insert(iterator __position,
                                 const_iterator __first,
                                 const_iterator __last)
{
  if (__first != __last)
  {
    size_type __n = 0;
    distance(__first, __last, __n);
    if (size_type(_M_end_of_storage - _M_finish) >= __n)
    {
      const size_type __elems_after = _M_finish - __position;
      iterator __old_finish = _M_finish;
      if (__elems_after > __n)
      {
        uninitialized_copy(_M_finish - __n, _M_finish, _M_finish);
        _M_finish += __n;
        copy_backward(__position, __old_finish - __n, __old_finish);
        copy(__first, __last, __position);
      }
      else
      {
        uninitialized_copy(__first + __elems_after, __last, _M_finish);
        _M_finish += __n - __elems_after;
        uninitialized_copy(__position, __old_finish, _M_finish);
        _M_finish += __elems_after;
        copy(__first, __first + __elems_after, __position);
      }
    }
    else
    {
      const size_type __old_size = size();
      const size_type __len = __old_size + max(__old_size, __n);
      iterator __new_start = _M_allocate(__len);
      iterator __new_finish = __new_start;
      __STL_TRY
      {
        __new_finish = uninitialized_copy(_M_start, __position, __new_start);
        __new_finish = uninitialized_copy(__first, __last, __new_finish);
        __new_finish = uninitialized_copy(__position, _M_finish, __new_finish);
      }
      __STL_UNWIND((destroy(__new_start, __new_finish),
                    _M_deallocate(__new_start, __len)));
      destroy(_M_start, _M_finish);
      _M_deallocate(_M_start, _M_end_of_storage - _M_start);
      _M_start = __new_start;
      _M_finish = __new_finish;
      _M_end_of_storage = __new_start + __len;
    }
  }
}

#endif /* __STL_MEMBER_TEMPLATES */

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#pragma reset woff 1375
#endif

__STL_END_NAMESPACE

#endif /* __SGI_STL_INTERNAL_VECTOR_H */

// Local Variables:
// mode:C++
// End:
