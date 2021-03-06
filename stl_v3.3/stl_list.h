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

#ifndef __SGI_STL_INTERNAL_LIST_H
#define __SGI_STL_INTERNAL_LIST_H

#include <concept_checks.h>

__STL_BEGIN_NAMESPACE

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#pragma set woff 1375
#endif

// 将连接的链子提取出来了
struct _List_node_base
{
  _List_node_base *_M_next;
  _List_node_base *_M_prev;
};

// 然后继承
// 这样链子就有了数据
template <class _Tp>
struct _List_node : public _List_node_base
{
  _Tp _M_data;
};

// 简单的封装访问.
struct _List_iterator_base
{
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef bidirectional_iterator_tag iterator_category;

  // 链子
  _List_node_base *_M_node;

  _List_iterator_base(_List_node_base *__x) : _M_node(__x) {}
  _List_iterator_base() {}

  void _M_incr() { _M_node = _M_node->_M_next; }
  void _M_decr() { _M_node = _M_node->_M_prev; }

  bool operator==(const _List_iterator_base &__x) const
  {
    return _M_node == __x._M_node;
  }
  bool operator!=(const _List_iterator_base &__x) const
  {
    return _M_node != __x._M_node;
  }
};

// 这层是封装了什么？封装了运算符？
template <class _Tp, class _Ref, class _Ptr>
struct _List_iterator : public _List_iterator_base
{
  typedef _List_iterator<_Tp, _Tp &, _Tp *> iterator;
  typedef _List_iterator<_Tp, const _Tp &, const _Tp *> const_iterator;
  typedef _List_iterator<_Tp, _Ref, _Ptr> _Self;

  typedef _Tp value_type;
  typedef _Ptr pointer;
  typedef _Ref reference;
  typedef _List_node<_Tp> _Node;

  _List_iterator(_Node *__x) : _List_iterator_base(__x) {}
  _List_iterator() {}
  // 这是个啥？表示迭代器？但是没有点运算符的啊。
  // 是_List_iterator_base？
  _List_iterator(const iterator &__x) : _List_iterator_base(__x._M_node) {}

  // 这里_M_node在哪里？是_Node吧？
  reference operator*() const { return ((_Node *)_M_node)->_M_data; }

#ifndef __SGI_STL_NO_ARROW_OPERATOR
  // 直接返回的&(operator*())，嗯，转换为值，然后再取地址？
  // ->直接指向了值
  pointer operator->() const { return &(operator*()); }
#endif /* __SGI_STL_NO_ARROW_OPERATOR */

  _Self &operator++()
  {
    this->_M_incr();
    return *this;
  }
  _Self operator++(int)
  {
    _Self __tmp = *this;
    this->_M_incr();
    return __tmp;
  }
  _Self &operator--()
  {
    this->_M_decr();
    return *this;
  }
  _Self operator--(int)
  {
    _Self __tmp = *this;
    this->_M_decr();
    return __tmp;
  }
};

#ifndef __STL_CLASS_PARTIAL_SPECIALIZATION

inline bidirectional_iterator_tag
iterator_category(const _List_iterator_base &)
{
  return bidirectional_iterator_tag();
}

template <class _Tp, class _Ref, class _Ptr>
inline _Tp *
value_type(const _List_iterator<_Tp, _Ref, _Ptr> &)
{
  return 0;
}

inline ptrdiff_t *
distance_type(const _List_iterator_base &)
{
  return 0;
}

#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */

// Base class that encapsulates details of allocators.  Three cases:
// an ordinary standard-conforming allocator, a standard-conforming
// allocator with no non-static data, and an SGI-style allocator.
// This complexity is necessary only because we're worrying about backward
// compatibility and because we want to avoid wasting storage on an
// allocator instance if it isn't necessary.

#ifdef __STL_USE_STD_ALLOCATORS

// Base for general standard-conforming allocators.
// 什么分配的内存分配器
template <class _Tp, class _Allocator, bool _IsStatic>
class _List_alloc_base
{
public:
  typedef typename _Alloc_traits<_Tp, _Allocator>::allocator_type
      allocator_type;
  allocator_type get_allocator() const { return _Node_allocator; }

  _List_alloc_base(const allocator_type &__a) : _Node_allocator(__a) {}

protected:
  // 就是添加和析构的函数
  // 只分配内存，用在哪里不知道。
  _List_node<_Tp> *_M_get_node()
  {
    return _Node_allocator.allocate(1);
  }
  void _M_put_node(_List_node<_Tp> *__p)
  {
    _Node_allocator.deallocate(__p, 1);
  }

protected:
  // 这里定义了名字啊.不是保存了
  typename _Alloc_traits<_List_node<_Tp>, _Allocator>::allocator_type
      _Node_allocator;
  // 这个_M_node又是什么啊？也没用啊。
  _List_node<_Tp> *_M_node;
};

// Specialization for instanceless allocators.

template <class _Tp, class _Allocator>
class _List_alloc_base<_Tp, _Allocator, true>
{
public:
  typedef typename _Alloc_traits<_Tp, _Allocator>::allocator_type
      allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }

  _List_alloc_base(const allocator_type &) {}

protected:
  typedef typename _Alloc_traits<_List_node<_Tp>, _Allocator>::_Alloc_type
      _Alloc_type;
  _List_node<_Tp> *_M_get_node() { return _Alloc_type::allocate(1); }
  void _M_put_node(_List_node<_Tp> *__p) { _Alloc_type::deallocate(__p, 1); }

protected:
  // 也是.干啥用的啊
  _List_node<_Tp> *_M_node;
};

// 单个节点的分配和析构?
template <class _Tp, class _Alloc>
class _List_base
    : public _List_alloc_base<_Tp, _Alloc,
                              _Alloc_traits<_Tp, _Alloc>::_S_instanceless>
{
public:
  typedef _List_alloc_base<_Tp, _Alloc,
                           _Alloc_traits<_Tp, _Alloc>::_S_instanceless>
      _Base;
  typedef typename _Base::allocator_type allocator_type;

  _List_base(const allocator_type &__a) : _Base(__a)
  {
    // 一个节点初始化
    _M_node = _M_get_node();
    _M_node->_M_next = _M_node;
    _M_node->_M_prev = _M_node;
  }
  ~_List_base()
  {
    clear();
    _M_put_node(_M_node);
  }

  void clear();
};

#else /* __STL_USE_STD_ALLOCATORS */

template <class _Tp, class _Alloc>
class _List_base
{
public:
  typedef _Alloc allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }

  _List_base(const allocator_type &)
  {
    _M_node = _M_get_node();
    _M_node->_M_next = _M_node;
    _M_node->_M_prev = _M_node;
  }
  ~_List_base()
  {
    clear();
    _M_put_node(_M_node);
  }

  void clear();

protected:
  typedef simple_alloc<_List_node<_Tp>, _Alloc> _Alloc_type;
  _List_node<_Tp> *_M_get_node() { return _Alloc_type::allocate(1); }
  void _M_put_node(_List_node<_Tp> *__p) { _Alloc_type::deallocate(__p, 1); }

protected:
  _List_node<_Tp> *_M_node;
};

#endif /* __STL_USE_STD_ALLOCATORS */

template <class _Tp, class _Alloc>
void _List_base<_Tp, _Alloc>::clear()
{
  // 节点收尾相连.所以可以这样遍历.释放
  _List_node<_Tp> *__cur = (_List_node<_Tp> *)_M_node->_M_next;
  while (__cur != _M_node)
  {
    _List_node<_Tp> *__tmp = __cur;
    __cur = (_List_node<_Tp> *)__cur->_M_next;
    _Destroy(&__tmp->_M_data);
    _M_put_node(__tmp);
  }
  // 最后一个节点不释放.然后给再自己指向自己
  _M_node->_M_next = _M_node;
  _M_node->_M_prev = _M_node;
}

template <class _Tp, class _Alloc = __STL_DEFAULT_ALLOCATOR(_Tp)>
class list : protected _List_base<_Tp, _Alloc>
{
  // requirements:

  __STL_CLASS_REQUIRES(_Tp, _Assignable);

  typedef _List_base<_Tp, _Alloc> _Base;

protected:
  typedef void *_Void_pointer;

public:
  typedef _Tp value_type;
  typedef value_type *pointer;
  typedef const value_type *const_pointer;
  typedef value_type &reference;
  typedef const value_type &const_reference;
  typedef _List_node<_Tp> _Node;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  typedef typename _Base::allocator_type allocator_type;
  allocator_type get_allocator() const { return _Base::get_allocator(); }

public:
  typedef _List_iterator<_Tp, _Tp &, _Tp *> iterator;
  typedef _List_iterator<_Tp, const _Tp &, const _Tp *> const_iterator;

#ifdef __STL_CLASS_PARTIAL_SPECIALIZATION
  typedef reverse_iterator<const_iterator> const_reverse_iterator;
  typedef reverse_iterator<iterator> reverse_iterator;
#else  /* __STL_CLASS_PARTIAL_SPECIALIZATION */
  typedef reverse_bidirectional_iterator<const_iterator, value_type,
                                         const_reference, difference_type>
      const_reverse_iterator;
  typedef reverse_bidirectional_iterator<iterator, value_type, reference,
                                         difference_type>
      reverse_iterator;
#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */

protected:
#ifdef __STL_HAS_NAMESPACES
  using _Base::_M_node;
  using _Base::_M_put_node;
  using _Base::_M_get_node;
#endif /* __STL_HAS_NAMESPACES */

protected:
  _Node *_M_create_node(const _Tp &__x)
  {
    _Node *__p = _M_get_node();
    __STL_TRY
    {
      _Construct(&__p->_M_data, __x);
    }
    // 如果出问题就析构分配的空间
    __STL_UNWIND(_M_put_node(__p));
    return __p;
  }

  // 空的节点
  _Node *_M_create_node()
  {
    _Node *__p = _M_get_node();
    __STL_TRY
    {
      _Construct(&__p->_M_data);
    }
    __STL_UNWIND(_M_put_node(__p));
    return __p;
  }

public:
  // 默认构造函数，保存内存分配器
  explicit list(const allocator_type &__a = allocator_type()) : _Base(__a) {}

  // 迭代器，返回的是指针
  iterator begin() { return (_Node *)(_M_node->_M_next); }
  const_iterator begin() const { return (_Node *)(_M_node->_M_next); }

  iterator end() { return _M_node; }
  const_iterator end() const { return _M_node; }

  // 下面的迭代器确实是一个类类型的对象了、
  // 操作上和指针时不相同的，所以不能使用指针
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

  // 初始状态下自己的所有指针指向自己
  bool empty() const { return _M_node->_M_next == _M_node; }
  // 两个指针的举例，使用的++
  size_type size() const
  {
    size_type __result = 0;
    distance(begin(), end(), __result);
    return __result;
  }

  // 为啥是-1？哦。
  size_type max_size() const { return size_type(-1); }
  // 首元素，直接就是指针了
  reference front() { return *begin(); }
  // 常量引用，的函数，就是后面加上一个const啊
  const_reference front() const { return *begin(); }
  // back是end-1
  reference back() { return *(--end()); }
  const_reference back() const { return *(--end()); }
  // 狡猾直接交换首节点就行了
  void swap(list<_Tp, _Alloc> &__x) { __STD::swap(_M_node, __x._M_node); }

  // 插入效率，高啊
  iterator insert(iterator __position, const _Tp &__x)
  {
    // 先去分配一个节点
    _Node *__tmp = _M_create_node(__x);
    // 设置相应的节点
    __tmp->_M_next = __position._M_node;
    __tmp->_M_prev = __position._M_node->_M_prev;
    __position._M_node->_M_prev->_M_next = __tmp;
    __position._M_node->_M_prev = __tmp;
    return __tmp;
  }
  iterator insert(iterator __position) { return insert(__position, _Tp()); }
#ifdef __STL_MEMBER_TEMPLATES
  // Check whether it's an integral type.  If so, it's not an iterator.

  // 这应该是个范围插入，第一个是插入n个
  template <class _Integer>
  void _M_insert_dispatch(iterator __pos, _Integer __n, _Integer __x,
                          __true_type)
  {
    _M_fill_insert(__pos, (size_type)__n, (_Tp)__x);
  }
  // 这个是插入范围
  template <class _InputIterator>
  void _M_insert_dispatch(iterator __pos,
                          _InputIterator __first, _InputIterator __last,
                          __false_type);

  template <class _InputIterator>
  void insert(iterator __pos, _InputIterator __first, _InputIterator __last)
  {
    // 使用者个来判断是什么，然后调用效应的函数
    // 第三个参数是个标记.
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_insert_dispatch(__pos, __first, __last, _Integral());
  }

#else  /* __STL_MEMBER_TEMPLATES */
  void insert(iterator __position, const _Tp *__first, const _Tp *__last);
  void insert(iterator __position,
              const_iterator __first, const_iterator __last);
#endif /* __STL_MEMBER_TEMPLATES */
  void insert(iterator __pos, size_type __n, const _Tp &__x)
  {
    _M_fill_insert(__pos, __n, __x);
  }
  void _M_fill_insert(iterator __pos, size_type __n, const _Tp &__x);

  void push_front(const _Tp &__x) { insert(begin(), __x); }
  void push_front() { insert(begin()); }
  void push_back(const _Tp &__x) { insert(end(), __x); }
  void push_back() { insert(end()); }

  iterator erase(iterator __position)
  {
    // 设置节点
    _List_node_base *__next_node = __position._M_node->_M_next;
    _List_node_base *__prev_node = __position._M_node->_M_prev;
    _Node *__n = (_Node *)__position._M_node;
    __prev_node->_M_next = __next_node;
    __next_node->_M_prev = __prev_node;
    // 析构节点
    _Destroy(&__n->_M_data);
    // 析构内存
    _M_put_node(__n);
    // 返回下一个节点
    return iterator((_Node *)__next_node);
  }
  iterator erase(iterator __first, iterator __last);
  // 清楚，只剩一个节点
  void clear() { _Base::clear(); }

  void resize(size_type __new_size, const _Tp &__x);
  void resize(size_type __new_size) { this->resize(__new_size, _Tp()); }

  void pop_front() { erase(begin()); }
  void pop_back()
  {
    iterator __tmp = end();
    // 这个简洁啊。因为是个环
    erase(--__tmp);
  }
  list(size_type __n, const _Tp &__value,
       const allocator_type &__a = allocator_type())
      : _Base(__a)
  {
    insert(begin(), __n, __value);
  }
  explicit list(size_type __n)
      : _Base(allocator_type())
  {
    insert(begin(), __n, _Tp());
  }

#ifdef __STL_MEMBER_TEMPLATES

  // We don't need any dispatching tricks here, because insert does all of
  // that anyway.
  template <class _InputIterator>
  list(_InputIterator __first, _InputIterator __last,
       const allocator_type &__a = allocator_type())
      : _Base(__a)
  {
    insert(begin(), __first, __last);
  }

#else /* __STL_MEMBER_TEMPLATES */

  list(const _Tp *__first, const _Tp *__last,
       const allocator_type &__a = allocator_type())
      : _Base(__a)
  {
    this->insert(begin(), __first, __last);
  }
  list(const_iterator __first, const_iterator __last,
       const allocator_type &__a = allocator_type())
      : _Base(__a)
  {
    this->insert(begin(), __first, __last);
  }

#endif /* __STL_MEMBER_TEMPLATES */
  list(const list<_Tp, _Alloc> &__x) : _Base(__x.get_allocator())
  {
    insert(begin(), __x.begin(), __x.end());
  }

  // 析构函数是空的？
  // 析构的时候基类负责清楚
  ~list() {}

  list<_Tp, _Alloc> &operator=(const list<_Tp, _Alloc> &__x);

public:
  // assign(), a generalized assignment member function.  Two
  // versions: one that takes a count, and one that takes a range.
  // The range version is a member template, so we dispatch on whether
  // or not the type is an integer.

  void assign(size_type __n, const _Tp &__val) { _M_fill_assign(__n, __val); }

  void _M_fill_assign(size_type __n, const _Tp &__val);

#ifdef __STL_MEMBER_TEMPLATES

  template <class _InputIterator>
  void assign(_InputIterator __first, _InputIterator __last)
  {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_assign_dispatch(__first, __last, _Integral());
  }

  template <class _Integer>
  void _M_assign_dispatch(_Integer __n, _Integer __val, __true_type)
  {
    _M_fill_assign((size_type)__n, (_Tp)__val);
  }

  template <class _InputIterator>
  void _M_assign_dispatch(_InputIterator __first, _InputIterator __last,
                          __false_type);

#endif /* __STL_MEMBER_TEMPLATES */

protected:
  void transfer(iterator __position, iterator __first, iterator __last)
  {
    if (__position != __last)
    {
      // Remove [first, last) from its old position.
      __last._M_node->_M_prev->_M_next = __position._M_node;
      __first._M_node->_M_prev->_M_next = __last._M_node;
      __position._M_node->_M_prev->_M_next = __first._M_node;

      // Splice [first, last) into its new position.
      // 使用一个临时对象来代替一个
      // 该对象析构的时候析构掉剩余的
      _List_node_base *__tmp = __position._M_node->_M_prev;
      __position._M_node->_M_prev = __last._M_node->_M_prev;
      __last._M_node->_M_prev = __first._M_node->_M_prev;
      __first._M_node->_M_prev = __tmp;
    }
  }

public:
  void splice(iterator __position, list &__x)
  {
    if (!__x.empty())
      this->transfer(__position, __x.begin(), __x.end());
  }
  void splice(iterator __position, list &, iterator __i)
  {
    iterator __j = __i;
    ++__j;
    if (__position == __i || __position == __j)
      return;
    this->transfer(__position, __i, __j);
  }
  void splice(iterator __position, list &, iterator __first, iterator __last)
  {
    if (__first != __last)
      this->transfer(__position, __first, __last);
  }
  void remove(const _Tp &__value);
  void unique();
  void merge(list &__x);
  void reverse();
  void sort();

#ifdef __STL_MEMBER_TEMPLATES
  template <class _Predicate>
  void remove_if(_Predicate);
  template <class _BinaryPredicate>
  void unique(_BinaryPredicate);
  template <class _StrictWeakOrdering>
  void merge(list &, _StrictWeakOrdering);
  template <class _StrictWeakOrdering>
  void sort(_StrictWeakOrdering);
#endif /* __STL_MEMBER_TEMPLATES */
};

template <class _Tp, class _Alloc>
inline bool
operator==(const list<_Tp, _Alloc> &__x, const list<_Tp, _Alloc> &__y)
{
  typedef typename list<_Tp, _Alloc>::const_iterator const_iterator;
  const_iterator __end1 = __x.end();
  const_iterator __end2 = __y.end();

  const_iterator __i1 = __x.begin();
  const_iterator __i2 = __y.begin();
  // 然后挨个比对。
  while (__i1 != __end1 && __i2 != __end2 && *__i1 == *__i2)
  {
    ++__i1;
    ++__i2;
  }
  // 当最后到结尾的时候，在比对依次这个。
  return __i1 == __end1 && __i2 == __end2;
}

template <class _Tp, class _Alloc>
inline bool operator<(const list<_Tp, _Alloc> &__x,
                      const list<_Tp, _Alloc> &__y)
{
  return lexicographical_compare(__x.begin(), __x.end(),
                                 __y.begin(), __y.end());
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER

template <class _Tp, class _Alloc>
inline bool operator!=(const list<_Tp, _Alloc> &__x,
                       const list<_Tp, _Alloc> &__y)
{
  return !(__x == __y);
}

template <class _Tp, class _Alloc>
inline bool operator>(const list<_Tp, _Alloc> &__x,
                      const list<_Tp, _Alloc> &__y)
{
  return __y < __x;
}

template <class _Tp, class _Alloc>
inline bool operator<=(const list<_Tp, _Alloc> &__x,
                       const list<_Tp, _Alloc> &__y)
{
  return !(__y < __x);
}

template <class _Tp, class _Alloc>
inline bool operator>=(const list<_Tp, _Alloc> &__x,
                       const list<_Tp, _Alloc> &__y)
{
  return !(__x < __y);
}

template <class _Tp, class _Alloc>
inline void
swap(list<_Tp, _Alloc> &__x, list<_Tp, _Alloc> &__y)
{
  __x.swap(__y);
}

#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */

#ifdef __STL_MEMBER_TEMPLATES

template <class _Tp, class _Alloc>
template <class _InputIter>
void list<_Tp, _Alloc>::_M_insert_dispatch(iterator __position,
                                           _InputIter __first, _InputIter __last,
                                           __false_type)
{
  for (; __first != __last; ++__first)
    insert(__position, *__first);
}

#else /* __STL_MEMBER_TEMPLATES */

template <class _Tp, class _Alloc>
void list<_Tp, _Alloc>::insert(iterator __position,
                               const _Tp *__first, const _Tp *__last)
{
  for (; __first != __last; ++__first)
    insert(__position, *__first);
}

template <class _Tp, class _Alloc>
void list<_Tp, _Alloc>::insert(iterator __position,
                               const_iterator __first, const_iterator __last)
{
  for (; __first != __last; ++__first)
    insert(__position, *__first);
}

#endif /* __STL_MEMBER_TEMPLATES */

template <class _Tp, class _Alloc>
void list<_Tp, _Alloc>::_M_fill_insert(iterator __position,
                                       size_type __n, const _Tp &__x)
{
  for (; __n > 0; --__n)
    insert(__position, __x);
}

template <class _Tp, class _Alloc>
typename list<_Tp, _Alloc>::iterator list<_Tp, _Alloc>::erase(iterator __first,
                                                              iterator __last)
{
  while (__first != __last)
    erase(__first++);
  return __last;
}

template <class _Tp, class _Alloc>
void list<_Tp, _Alloc>::resize(size_type __new_size, const _Tp &__x)
{
  iterator __i = begin();
  size_type __len = 0;
  // 找到期望的最后节点，然后查处掉后面的
  for (; __i != end() && __len < __new_size; ++__i, ++__len)
    ;
  if (__len == __new_size)
    erase(__i, end());
  else // __i == end()
  // 如果不够，那么就添加。
    insert(end(), __new_size - __len, __x);
}

template <class _Tp, class _Alloc>
list<_Tp, _Alloc> &list<_Tp, _Alloc>::operator=(const list<_Tp, _Alloc> &__x)
{
  if (this != &__x)
  {
    iterator __first1 = begin();
    iterator __last1 = end();
    const_iterator __first2 = __x.begin();
    const_iterator __last2 = __x.end();
    // 赋值
    while (__first1 != __last1 && __first2 != __last2)
      *__first1++ = *__first2++;
      // 长处的部分释放，erase有析构对象和释放控价你的函数
    if (__first2 == __last2)
      erase(__first1, __last1);
    else
    // 不够的部分插入
      insert(__last1, __first2, __last2);
  }
  return *this;
}

template <class _Tp, class _Alloc>
void list<_Tp, _Alloc>::_M_fill_assign(size_type __n, const _Tp &__val)
{
  iterator __i = begin();
  for (; __i != end() && __n > 0; ++__i, --__n)
    *__i = __val;
  if (__n > 0)
    insert(end(), __n, __val);
  else
    erase(__i, end());
}

#ifdef __STL_MEMBER_TEMPLATES

template <class _Tp, class _Alloc>
template <class _InputIter>
void list<_Tp, _Alloc>::_M_assign_dispatch(_InputIter __first2, _InputIter __last2,
                                           __false_type)
{
  iterator __first1 = begin();
  iterator __last1 = end();
  for (; __first1 != __last1 && __first2 != __last2; ++__first1, ++__first2)
    *__first1 = *__first2;
  if (__first2 == __last2)
    erase(__first1, __last1);
  else
    insert(__last1, __first2, __last2);
}

#endif /* __STL_MEMBER_TEMPLATES */

template <class _Tp, class _Alloc>
void list<_Tp, _Alloc>::remove(const _Tp &__value)
{
  iterator __first = begin();
  iterator __last = end();
  // 遍历，然后擦除
  while (__first != __last)
  {
    iterator __next = __first;
    ++__next;
    if (*__first == __value)
      erase(__first);
    __first = __next;
  }
}

// 可以实现?
// 从代码上看,貌似这是两个相邻的不能重复啊
template <class _Tp, class _Alloc>
void list<_Tp, _Alloc>::unique()
{
  iterator __first = begin();
  iterator __last = end();
  if (__first == __last)
    return;
  iterator __next = __first;
  while (++__next != __last)
  {
    if (*__first == *__next)
      erase(__next);
    else
      __first = __next;
    __next = __first;
  }
}

template <class _Tp, class _Alloc>
void list<_Tp, _Alloc>::merge(list<_Tp, _Alloc> &__x)
{
  iterator __first1 = begin();
  iterator __last1 = end();
  iterator __first2 = __x.begin();
  iterator __last2 = __x.end();
  while (__first1 != __last1 && __first2 != __last2)
    if (*__first2 < *__first1)
    {
      iterator __next = __first2;
      transfer(__first1, __first2, ++__next);
      __first2 = __next;
    }
    else
      ++__first1;
  if (__first2 != __last2)
    transfer(__last1, __first2, __last2);
}

inline void __List_base_reverse(_List_node_base *__p)
{
  _List_node_base *__tmp = __p;
  do
  {
    __STD::swap(__tmp->_M_next, __tmp->_M_prev);
    __tmp = __tmp->_M_prev; // Old next node is now prev.
  } while (__tmp != __p);
}

template <class _Tp, class _Alloc>
inline void list<_Tp, _Alloc>::reverse()
{
  __List_base_reverse(this->_M_node);
}

// 这个排序函数怎么实现的??
// ***********************先留下吧*************
template <class _Tp, class _Alloc>
void list<_Tp, _Alloc>::sort()
{
  // Do nothing if the list has length 0 or 1.
  if (_M_node->_M_next != _M_node && _M_node->_M_next->_M_next != _M_node)
  {
    list<_Tp, _Alloc> __carry;
    list<_Tp, _Alloc> __counter[64];
    int __fill = 0;
    while (!empty())
    {
      __carry.splice(__carry.begin(), *this, begin());
      int __i = 0;
      while (__i < __fill && !__counter[__i].empty())
      {
        __counter[__i].merge(__carry);
        __carry.swap(__counter[__i++]);
      }
      __carry.swap(__counter[__i]);
      if (__i == __fill)
        ++__fill;
    }

    for (int __i = 1; __i < __fill; ++__i)
      __counter[__i].merge(__counter[__i - 1]);
    swap(__counter[__fill - 1]);
  }
}

#ifdef __STL_MEMBER_TEMPLATES

template <class _Tp, class _Alloc>
template <class _Predicate>
void list<_Tp, _Alloc>::remove_if(_Predicate __pred)
{
  iterator __first = begin();
  iterator __last = end();
  while (__first != __last)
  {
    iterator __next = __first;
    ++__next;
    // 条件函数
    if (__pred(*__first))
      erase(__first);
    __first = __next;
  }
}

// 这个函数接受一个什么东西?
template <class _Tp, class _Alloc>
template <class _BinaryPredicate>
void list<_Tp, _Alloc>::unique(_BinaryPredicate __binary_pred)
{
  iterator __first = begin();
  iterator __last = end();
  if (__first == __last)
    return;
  iterator __next = __first;
  while (++__next != __last)
  {
    if (__binary_pred(*__first, *__next))
      erase(__next);
    else
      __first = __next;
    __next = __first;
  }
}

template <class _Tp, class _Alloc>
template <class _StrictWeakOrdering>
void list<_Tp, _Alloc>::merge(list<_Tp, _Alloc> &__x,
                              _StrictWeakOrdering __comp)
{
  iterator __first1 = begin();
  iterator __last1 = end();
  iterator __first2 = __x.begin();
  iterator __last2 = __x.end();
  while (__first1 != __last1 && __first2 != __last2)
    if (__comp(*__first2, *__first1))
    {
      iterator __next = __first2;
      transfer(__first1, __first2, ++__next);
      __first2 = __next;
    }
    else
      ++__first1;
  if (__first2 != __last2)
    transfer(__last1, __first2, __last2);
}

template <class _Tp, class _Alloc>
template <class _StrictWeakOrdering>
void list<_Tp, _Alloc>::sort(_StrictWeakOrdering __comp)
{
  // Do nothing if the list has length 0 or 1.
  if (_M_node->_M_next != _M_node && _M_node->_M_next->_M_next != _M_node)
  {
    list<_Tp, _Alloc> __carry;
    list<_Tp, _Alloc> __counter[64];
    int __fill = 0;
    while (!empty())
    {
      __carry.splice(__carry.begin(), *this, begin());
      int __i = 0;
      while (__i < __fill && !__counter[__i].empty())
      {
        __counter[__i].merge(__carry, __comp);
        __carry.swap(__counter[__i++]);
      }
      __carry.swap(__counter[__i]);
      if (__i == __fill)
        ++__fill;
    }

    for (int __i = 1; __i < __fill; ++__i)
      __counter[__i].merge(__counter[__i - 1], __comp);
    swap(__counter[__fill - 1]);
  }
}

#endif /* __STL_MEMBER_TEMPLATES */

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#pragma reset woff 1375
#endif

__STL_END_NAMESPACE

#endif /* __SGI_STL_INTERNAL_LIST_H */

// Local Variables:
// mode:C++
// End:
