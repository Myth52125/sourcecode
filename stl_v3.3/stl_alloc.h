/*
 * Copyright (c) 1996-1997
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

#ifndef __SGI_STL_INTERNAL_ALLOC_H
#define __SGI_STL_INTERNAL_ALLOC_H

#ifdef __SUNPRO_CC
#define __PRIVATE public
// Extra access restrictions prevent us from really making some things
// private.
#else
#define __PRIVATE private
#endif

#ifdef __STL_STATIC_TEMPLATE_MEMBER_BUG
#define __USE_MALLOC
#endif

// This implements some standard node allocators.  These are
// NOT the same as the allocators in the C++ draft standard or in
// in the original STL.  They do not encapsulate different pointer
// types; indeed we assume that there is only one pointer type.
// The allocation primitives are intended to allocate individual objects,
// not larger arenas as with the original STL allocators.

#ifndef __THROW_BAD_ALLOC
#if defined(__STL_NO_BAD_ALLOC) || !defined(__STL_USE_EXCEPTIONS)
#include <stdio.h>
#include <stdlib.h>
#define __THROW_BAD_ALLOC             \
  fprintf(stderr, "out of memory\n"); \
  exit(1)
#else /* Standard conforming out-of-memory handling */
#include <new>
#define __THROW_BAD_ALLOC throw std::bad_alloc()
#endif
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifndef __RESTRICT
#define __RESTRICT
#endif

#ifdef __STL_THREADS
#include <stl_threads.h>
#define __NODE_ALLOCATOR_THREADS true
#ifdef __STL_SGI_THREADS
// We test whether threads are in use before locking.
// Perhaps this should be moved into stl_threads.h, but that
// probably makes it harder to avoid the procedure call when
// it isn't needed.
extern "C" {
extern int __us_rsthread_malloc;
}
// The above is copied from malloc.h.  Including <malloc.h>
// would be cleaner but fails with certain levels of standard
// conformance.
#define __NODE_ALLOCATOR_LOCK                 \
  if (threads && __us_rsthread_malloc)        \
  {                                           \
    _S_node_allocator_lock._M_acquire_lock(); \
  }
#define __NODE_ALLOCATOR_UNLOCK               \
  if (threads && __us_rsthread_malloc)        \
  {                                           \
    _S_node_allocator_lock._M_release_lock(); \
  }
#else /* !__STL_SGI_THREADS */
#define __NODE_ALLOCATOR_LOCK                   \
  {                                             \
    if (threads)                                \
      _S_node_allocator_lock._M_acquire_lock(); \
  }
#define __NODE_ALLOCATOR_UNLOCK                 \
  {                                             \
    if (threads)                                \
      _S_node_allocator_lock._M_release_lock(); \
  }
#endif
#else
//  Thread-unsafe
#define __NODE_ALLOCATOR_LOCK
#define __NODE_ALLOCATOR_UNLOCK
#define __NODE_ALLOCATOR_THREADS false
#endif

__STL_BEGIN_NAMESPACE

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#endif

// Malloc-based allocator.  Typically slower than default alloc below.
// Typically thread-safe and more storage efficient.
#ifdef __STL_STATIC_TEMPLATE_MEMBER_BUG
#ifdef __DECLARE_GLOBALS_HERE
void (*__malloc_alloc_oom_handler)() = 0;
// g++ 2.7.2 does not handle static template data members.
#else

extern void (*__malloc_alloc_oom_handler)();
#endif
#endif

/*
该类所有的方法和成员几乎都是静态，也就是说，嗯，这个类
在程序一开始就初始化了。

这个类就是分配内存空间啊，如果分配不成功就循环分配。

同时使用一个分配函数，该函数干了啥不知道啊。

分配空间使用的是malloc，重新分配空间使用realloc

单为什么要用模板，里面没用到__inst啊？？
*/
template <int __inst>
class __malloc_alloc_template
{

private:
  static void *_S_oom_malloc(size_t);
  static void *_S_oom_realloc(void *, size_t);

#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
  static void (*__malloc_alloc_oom_handler)();
#endif

public:
  //分配内存
  static void *allocate(size_t __n)
  {
    //使用malloc()分配
    void *__result = malloc(__n);
    //使用值在前而不是变量在前的方式判断。嗯，以后要借鉴
    //如果分配失败就循环进行分配
    //但是__malloc_alloc_oom_handler不存在的话就报错，这个函数在上面extern的
    if (0 == __result)
      __result = _S_oom_malloc(__n);
    return __result;
  }

  static void deallocate(void *__p, size_t /* __n */)
  {
    //就free了？使用的malloc和free
    free(__p);
  }

  //重新分配内存
  static void *reallocate(void *__p, size_t /* old_sz */, size_t __new_sz)
  {
    //realloc()
    //先判断当前的指针是否有足够的连续空间，如果有，扩大mem_address指向的地址，并且将mem_address返回
    // 如果空间不够，先按照newsize指定的大小分配空间，将原有数据从头到尾拷贝到新分配的内存区域
    // 而后释放原来mem_address所指内存区域
    // （注意：原来指针是自动释放，不需要手动使用free，该函数内部进行了free）
    // 同时返回新分配的内存区域的首地址。即重新分配存储器块的地址。
    void *__result = realloc(__p, __new_sz);
    if (0 == __result)
      __result = _S_oom_realloc(__p, __new_sz);
    return __result;
  }

  //void (* __set_malloc_handler(void (*__f)()))()是一个函数，
  // 它接受一个形式为void (*_)()的函数指针作参数，
  // 并且返回一个形式为void (*)()的函数指针。
  // 也就是void 和最后一个(),表示返回值
  // * __set_malloc_handler(void (*__f)())这才是函数名和参数
  // void (*__f)()这就是参数
  static void (*__set_malloc_handler(void (*__f)()))()
  {
    //将参数设置为新的分配内存的函数。
    void (*__old)() = __malloc_alloc_oom_handler;
    __malloc_alloc_oom_handler = __f;
    //旧的返回。
    return (__old);
  }
};

// malloc_alloc out-of-memory handling

#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
template <int __inst>
void (*__malloc_alloc_template<__inst>::__malloc_alloc_oom_handler)() = 0;
#endif

template <int __inst>
void *
__malloc_alloc_template<__inst>::_S_oom_malloc(size_t __n)
{
  void (*__my_malloc_handler)();
  void *__result;

  //for循环去
  for (;;)
  {
    //这里取这个函数。
    __my_malloc_handler = __malloc_alloc_oom_handler;
    //#    define __THROW_BAD_ALLOC fprintf(stderr, "out of memory\n"); exit(1)
    //如果没定义，就退出程序了
    if (0 == __my_malloc_handler)
    {
      __THROW_BAD_ALLOC;
    }
    //然后执行这个函数。
    (*__my_malloc_handler)();
    //循环分配
    __result = malloc(__n);
    //如果分配成功就返回。
    if (__result)
      return (__result);
  }
}

//reallocate失败以后的循环分配函数。和_S_oom_malloc差不多的形式
template <int __inst>
void *__malloc_alloc_template<__inst>::_S_oom_realloc(void *__p, size_t __n)
{
  void (*__my_malloc_handler)();
  void *__result;

  for (;;)
  {
    __my_malloc_handler = __malloc_alloc_oom_handler;
    if (0 == __my_malloc_handler)
    {
      __THROW_BAD_ALLOC;
    }
    (*__my_malloc_handler)();
    //使用realloc
    __result = realloc(__p, __n);
    if (__result)
      return (__result);
  }
}

typedef __malloc_alloc_template<0> malloc_alloc;
/*
_Alloc应该就是__malloc_alloc_template这个类了。

这个类干了个啥？就是封装了下。封装了一个类型，一个分配器类。
全是静态成员
*/
template <class _Tp, class _Alloc>
class simple_alloc
{

public:
  static _Tp *allocate(size_t __n)
  {
    return 0 == __n ? 0 : (_Tp *)_Alloc::allocate(__n * sizeof(_Tp));
  }
  //如果没有大小，就分配一个。
  static _Tp *allocate(void)
  {
    return (_Tp *)_Alloc::allocate(sizeof(_Tp));
  }
  //__malloc_alloc_template的deallocate，没有用到第二个参数啊。
  static void deallocate(_Tp *__p, size_t __n)
  {
    if (0 != __n)
      _Alloc::deallocate(__p, __n * sizeof(_Tp));
  }
  static void deallocate(_Tp *__p)
  {
    _Alloc::deallocate(__p, sizeof(_Tp));
  }
};

// Allocator adaptor to check size arguments for debugging.
// Reports errors using assert.  Checking can be disabled with
// NDEBUG, but it's far better to just use the underlying allocator
// instead when no checking is desired.
// There is some evidence that this can confuse Purify.
/*

这个类有点看不懂
*/
template <class _Alloc>
class debug_alloc
{

private:
  enum
  {
    _S_extra = 8
  }; // Size of space used to store size.  Note
     // that this must be large enough to preserve
     // alignment.

public:
  static void *allocate(size_t __n)
  {
    //保证不会分配为0的内存空间, 而且要保证内存对齐,所以加上_S_extra=8
    char *__result = (char *)_Alloc::allocate(__n + (int)_S_extra);
    *(size_t *)__result = __n;
    // 把分配内存的最前面设置成n的大小, 用于后面校验
    // 内存对齐的作用就是保护前面extra大小的数据不被修改
    return __result + (int)_S_extra;
  }

  static void deallocate(void *__p, size_t __n)
  {
    char *__real_p = (char *)__p - (int)_S_extra;
    //如果*(size_t *)real_p != n则肯定发生向前越界
    assert(*(size_t *)__real_p == __n);
    _Alloc::deallocate(__real_p, __n + (int)_S_extra);
  }

  static void *reallocate(void *__p, size_t __old_sz, size_t __new_sz)
  {
    char *__real_p = (char *)__p - (int)_S_extra;
    assert(*(size_t *)__real_p == __old_sz);
    char *__result = (char *)
        _Alloc::reallocate(__real_p, __old_sz + (int)_S_extra,
                           __new_sz + (int)_S_extra);
    *(size_t *)__result = __new_sz;
    return __result + (int)_S_extra;
  }
};

#ifdef __USE_MALLOC

typedef malloc_alloc alloc;
typedef malloc_alloc single_client_alloc;

#else

// Default node allocator.
// With a reasonable compiler, this should be roughly as fast as the
// original STL class-specific allocators, but with less fragmentation.
// Default_alloc_template parameters are experimental and MAY
// DISAPPEAR in the future.  Clients should just use alloc for now.
//
// Important implementation properties:
// 1. If the client request an object of size > _MAX_BYTES, the resulting
//    object will be obtained directly from malloc.
// 2. In all other cases, we allocate an object of size exactly
//    _S_round_up(requested_size).  Thus the client has enough size
//    information that we can return the object to the proper free list
//    without permanently losing part of the object.
//

// The first template parameter specifies whether more than one thread
// may use this allocator.  It is safe to allocate an object from
// one instance of a default_alloc and deallocate it with another
// one.  This effectively transfers its ownership to the second one.
// This may have undesirable effects on reference locality.
// The second parameter is unreferenced and serves only to allow the
// creation of multiple default_alloc instances.
// Node that containers built on different allocator instances have
// different types, limiting the utility of this approach.

#if defined(__SUNPRO_CC) || defined(__GNUC__)
// breaks if we make these template class members:
enum
{
  _ALIGN = 8
};
enum
{
  _MAX_BYTES = 128
};
enum
{
  _NFREELISTS = 16
}; // _MAX_BYTES/_ALIGN
#endif
/*
这个类厉害了啊
是一个二级分配器
也是主要的分配器
*/
template <bool threads, int inst>
class __default_alloc_template
{

private:
// Really we should use static const int x = N
// instead of enum { x = N }, but few compilers accept the former.
#if !(defined(__SUNPRO_CC) || defined(__GNUC__))
  enum
  {
    _ALIGN = 8
  };
  enum
  {
    _MAX_BYTES = 128
  };
  enum
  {
    _NFREELISTS = 16
  }; // _MAX_BYTES/_ALIGN
#endif
  //内存对齐的惯用手法。先对齐-1，然后抹去后面的对齐-1位
  //另一种对齐方法((((bytes) + _ALIGN - 1) * _ALIGN) / _ALIGN)
  //使用除法的性质，将小数部分抹去。
  //根据请求的地址，返回一个从该地址开始往后的第一个当内存对齐位置，包括该地址
  //或者是将一个值，调整到对齐的大小。
  static size_t
  _S_round_up(size_t __bytes)
  {
    return (((__bytes) + (size_t)_ALIGN - 1) & ~((size_t)_ALIGN - 1));
  }

  __PRIVATE :
      // 在一个“联合”内可以定义多种不同的数据类型， 一个被说明为该“联合”类型的变量中，
      // 允许装入该“联合”所定义的任何一种数据，这些数据共享同一段内存，
      // 已达到节省空间的目的（还有一个节省空间的类型：位域）。
      // 这是一个非常特殊的地方，也是联合的特征。
      // 另外，同struct一样，联合默认访问权限也是公有的，并且，也具有成员函数。
      // 而在“联合”中，各成员共享一段内存空间， 一个联合变量的长度等于各成员中最长的长度
      // 所谓的共享不是指把多个成员同时装入一个联合变量内，
      // 而是指该联合变量可被赋予任一成员值，但每次只能赋一种值， 赋入新值则冲去旧值

  union _Obj {
    union _Obj *_M_free_list_link;
    //这个根本就没用到
    char _M_client_data[1]; /* The client sees this.        */
  };

private:
//这里定义了一个数组，这个数组有volatile属性，同时，大小为16*一个指针大小.
#if defined(__SUNPRO_CC) || defined(__GNUC__) || defined(__HP_aCC)
  static _Obj *__STL_VOLATILE _S_free_list[];
// Specifying a size results in duplicate def for 4.1
#else
  static _Obj *__STL_VOLATILE _S_free_list[_NFREELISTS];
#endif
  //返回__bytes，所需要分配的内存对齐的块的大小。
  static size_t _S_freelist_index(size_t __bytes)
  {
    return (((__bytes) + (size_t)_ALIGN - 1) / (size_t)_ALIGN - 1);
  }

  static void *_S_refill(size_t __n);
  static char *_S_chunk_alloc(size_t __size, int &__nobjs);

  //内存池起点，终点，已经分配了的大小。
  static char *_S_start_free;
  static char *_S_end_free;
  static size_t _S_heap_size;

//没看懂****************************************
#ifdef __STL_THREADS
  static _STL_mutex_lock _S_node_allocator_lock;
#endif

  // It would be nice to use _STL_auto_lock here.  But we
  // don't need the NULL check.  And we do need a test whether
  // threads have actually been started.
  class _Lock;
  friend class _Lock;
  class _Lock
  {
  public:
    _Lock() { __NODE_ALLOCATOR_LOCK; }
    ~_Lock() { __NODE_ALLOCATOR_UNLOCK; }
  };

public:
  /* __n must be > 0      */
  static void *allocate(size_t __n)
  {
    void *__ret = 0;
    //大于预定的大小，直接使用，分配器在堆上分配 128
    if (__n > (size_t)_MAX_BYTES)
    {
      __ret = malloc_alloc::allocate(__n);
    }
    else
    {
      //首先根据请求的内存大小，调整为一个数组中有的块的大小的值。
      //然后取得这个数组中的第一块内存块。
      _Obj *__STL_VOLATILE *__my_free_list = _S_free_list + _S_freelist_index(__n);
// Acquire the lock here with a constructor call.
// This ensures that it is released in exit or during stack
// unwinding.
#ifndef _NOTHREADS
      /*REFERENCED*/
      _Lock __lock_instance;
#endif
      //如果这块内存块不存在，也就是对应大小的内存块没有分配，那么先分配
      _Obj *__RESTRICT __result = *__my_free_list;
      if (__result == 0)
        __ret = _S_refill(_S_round_up(__n));
      else
      {
        //如果分配了，那么就将这块内存块取出来
        //同时数组中的指针设置为，下一块未用的内存块
        //是有可能为nullptr的
        *__my_free_list = __result->_M_free_list_link;
        //然后返回这块内存块。
        __ret = __result;
      }
    }

    return __ret;
  };
  //没看懂****************************************
  /* __p may not be 0 */
  static void deallocate(void *__p, size_t __n)
  {
    //如果内存块大于128，那么表明是在一级分配器上分配的
    //由一级分配器析构
    if (__n > (size_t)_MAX_BYTES)
      malloc_alloc::deallocate(__p, __n);
    else
    {
      //否则，就需要计算内存块的大小，然后将内存块，放回到
      //对应数组中的第一个块。
      _Obj *__STL_VOLATILE *__my_free_list = _S_free_list + _S_freelist_index(__n);
      _Obj *__q = (_Obj *)__p;

// acquire lock
#ifndef _NOTHREADS
      /*REFERENCED*/
      _Lock __lock_instance;
#endif /* _NOTHREADS */
      //刚回收的内存块指向数组的第一个内存块
      __q->_M_free_list_link = *__my_free_list;
      //然后数组的第一个元素再指向刚回收的第一个内存块，形成回收。
      *__my_free_list = __q;
      // lock is released here
    }
  }

  static void *reallocate(void *__p, size_t __old_sz, size_t __new_sz);
};

typedef __default_alloc_template<__NODE_ALLOCATOR_THREADS, 0> alloc;
typedef __default_alloc_template<false, 0> single_client_alloc;

template <bool __threads, int __inst>
inline bool operator==(const __default_alloc_template<__threads, __inst> &,
                       const __default_alloc_template<__threads, __inst> &)
{
  return true;
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
template <bool __threads, int __inst>
inline bool operator!=(const __default_alloc_template<__threads, __inst> &,
                       const __default_alloc_template<__threads, __inst> &)
{
  return false;
}
#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */

/* We allocate memory in large chunks in order to avoid fragmenting     */
/* the malloc heap too much.                                            */
/* We assume that size is properly aligned.                             */
/* We hold the allocation lock.                                         */

template <bool __threads, int __inst>
char *
__default_alloc_template<__threads, __inst>::_S_chunk_alloc(size_t __size,
                                                            int &__nobjs)
{
  char *__result;
  //将要分配的空间
  size_t __total_bytes = __size * __nobjs;
  //剩余可以分配的空间，刚开始的时候都是0
  size_t __bytes_left = _S_end_free - _S_start_free;

  //剩余空间足够，那么就直接分配出去
  if (__bytes_left >= __total_bytes)
  {
    __result = _S_start_free;
    //改变可分配指针的位置
    _S_start_free += __total_bytes;
    return (__result);
  }
  else if (__bytes_left >= __size)
  {
    //如果不能分配完整请求的内存，那么能分配几个分配几个
    //剩余空间能够分配的个数
    //__nobjs是个引用，可以传出去
    __nobjs = (int)(__bytes_left / __size);
    //剩下的内存能分配多少就分配多少，多出来的没分配。
    __total_bytes = __size * __nobjs;
    __result = _S_start_free;
    _S_start_free += __total_bytes;
    return (__result);
  }
  else
  {
    //内存池剩余的空间一块都分配不上了。
    //计算所需内存大小的两倍+之前分配的大小
    size_t __bytes_to_get =
        2 * __total_bytes + _S_round_up(_S_heap_size >> 4);
    // Try to make use of the left-over piece.
    //如果已分配还剩余一点。
    if (__bytes_left > 0)
    {
      //计算剩余的这点能够一块放数组的那个内存块中
      _Obj *__STL_VOLATILE *__my_free_list =
          _S_free_list + _S_freelist_index(__bytes_left);

      //让后将剩余的内存挂在在内存块数组中。
      ((_Obj *)_S_start_free)->_M_free_list_link = *__my_free_list;
      *__my_free_list = (_Obj *)_S_start_free;
    }
    //然后重新分配请求内存大小*2+之前分配的大小
    _S_start_free = (char *)malloc(__bytes_to_get);
    //如果分配失败了。
    if (0 == _S_start_free)
    {
      size_t __i;
      _Obj *__STL_VOLATILE *__my_free_list;
      _Obj *__p;
      // Try to make do with what we have.  That can't
      // hurt.  We do not try smaller requests, since that tends
      // to result in disaster on multi-process machines.

      //检查一下，从该大于该内存块的内存块现有
      for (__i = __size;
           __i <= (size_t)_MAX_BYTES;
           __i += (size_t)_ALIGN)
      {
        __my_free_list = _S_free_list + _S_freelist_index(__i);
        __p = *__my_free_list;
        //如果有内存没分配。
        //那么从比请求内存块大的内存块中，调出一块来，
        //分配给内存池，然后再分配给请求的内存块。
        if (0 != __p)
        {
          *__my_free_list = __p->_M_free_list_link;
          _S_start_free = (char *)__p;
          _S_end_free = _S_start_free + __i;
          return (_S_chunk_alloc(__size, __nobjs));
          // Any leftover piece will eventually make it to the
          // right free list.
        }
      }
      //如果分配失败了，比这块内存打的内存还没有了，表示没有一点内存可以分配了。
      _S_end_free = 0; // In case of exception.
      //使用一级分配器来试一下，如果分配不到内存就直接抛出错误了？
      _S_start_free = (char *)malloc_alloc::allocate(__bytes_to_get);
      // This should either throw an
      // exception or remedy the situation.  Thus we assume it
      // succeeded.
    }
    _S_heap_size += __bytes_to_get;
    _S_end_free = _S_start_free + __bytes_to_get;
    //分配到足够的内存以后，再去调用，在内存空间上分配内存。
    return (_S_chunk_alloc(__size, __nobjs));
  }
}

/* Returns an object of size __n, and optionally adds to size __n free list.*/
/* We assume that __n is properly aligned.                                */
/* We hold the allocation lock.                                         */
template <bool __threads, int __inst>
void *
__default_alloc_template<__threads, __inst>::_S_refill(size_t __n)
{
  int __nobjs = 20;
  char *__chunk = _S_chunk_alloc(__n, __nobjs);
  _Obj *__STL_VOLATILE *__my_free_list;
  _Obj *__result;
  _Obj *__current_obj;
  _Obj *__next_obj;
  int __i;
  //保证返回不为0，如果没内存，在_S_chunk_alloc中就结束了
  //如果只返回一块，那么直接返回给调用者
  if (1 == __nobjs)
    return (__chunk);
  __my_free_list = _S_free_list + _S_freelist_index(__n);

  /* Build free list in chunk */
  //如果大于一块，那么第一块返回给调用者。
  __result = (_Obj *)__chunk;
  //可以分配的起始地址，因为之前的那块返回给用户了
  *__my_free_list = __next_obj = (_Obj *)(__chunk + __n);
  for (__i = 1;; __i++)
  {
    __current_obj = __next_obj;
    //将返回的空间进行划分。
    __next_obj = (_Obj *)((char *)__next_obj + __n);
    if (__nobjs - 1 == __i)
    {
      //判断都划分晚了
      __current_obj->_M_free_list_link = 0;
      break;
    }
    else
    {
      //递归向后添加，这里最后一块就是链表的尾部
      __current_obj->_M_free_list_link = __next_obj;
    }
  }
  //划分结束以后返回最后一块。
  return (__result);
}

template <bool threads, int inst>
void *
__default_alloc_template<threads, inst>::reallocate(void *__p,
                                                    size_t __old_sz,
                                                    size_t __new_sz)
{
  void *__result;
  size_t __copy_sz;

  if (__old_sz > (size_t)_MAX_BYTES && __new_sz > (size_t)_MAX_BYTES)
  {
    return (realloc(__p, __new_sz));
  }
  if (_S_round_up(__old_sz) == _S_round_up(__new_sz))
    return (__p);
  __result = allocate(__new_sz);
  __copy_sz = __new_sz > __old_sz ? __old_sz : __new_sz;
  //多了一个拷贝。也因此在这以后之前的指针和引用乱七八糟的都失效了
  memcpy(__result, __p, __copy_sz);
  //析构了之前的空间
  deallocate(__p, __old_sz);
  //返回了新空间
  return (__result);
}

#ifdef __STL_THREADS
template <bool __threads, int __inst>
_STL_mutex_lock
    __default_alloc_template<__threads, __inst>::_S_node_allocator_lock
        __STL_MUTEX_INITIALIZER;
#endif

template <bool __threads, int __inst>
char *__default_alloc_template<__threads, __inst>::_S_start_free = 0;

template <bool __threads, int __inst>
char *__default_alloc_template<__threads, __inst>::_S_end_free = 0;

template <bool __threads, int __inst>
size_t __default_alloc_template<__threads, __inst>::_S_heap_size = 0;

template <bool __threads, int __inst>
typename __default_alloc_template<__threads, __inst>::_Obj *__STL_VOLATILE
    __default_alloc_template<__threads, __inst>::_S_free_list[
#if defined(__SUNPRO_CC) || defined(__GNUC__) || defined(__HP_aCC)
        _NFREELISTS
#else
        __default_alloc_template<__threads, __inst>::_NFREELISTS
#endif
] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
// The 16 zeros are necessary to make version 4.1 of the SunPro
// compiler happy.  Otherwise it appears to allocate too little
// space for the array.

#endif /* ! __USE_MALLOC */

// This implements allocators as specified in the C++ standard.
//
// Note that standard-conforming allocators use many language features
// that are not yet widely implemented.  In particular, they rely on
// member templates, partial specialization, partial ordering of function
// templates, the typename keyword, and the use of the template keyword
// to refer to a template member of a dependent type.

#ifdef __STL_USE_STD_ALLOCATORS

template <class _Tp>
class allocator
{
  typedef alloc _Alloc; // The underlying allocator.
public:
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef _Tp *pointer;
  typedef const _Tp *const_pointer;
  typedef _Tp &reference;
  typedef const _Tp &const_reference;
  typedef _Tp value_type;

  template <class _Tp1>
  struct rebind
  {
    typedef allocator<_Tp1> other;
  };

  allocator() __STL_NOTHROW {}
  allocator(const allocator &) __STL_NOTHROW {}
  template <class _Tp1>
  allocator(const allocator<_Tp1> &) __STL_NOTHROW {}
  ~allocator() __STL_NOTHROW {}

  pointer address(reference __x) const { return &__x; }
  const_pointer address(const_reference __x) const { return &__x; }

  // __n is permitted to be 0.  The C++ standard says nothing about what
  // the return value is when __n == 0.
  _Tp *allocate(size_type __n, const void * = 0)
  {
    return __n != 0 ? static_cast<_Tp *>(_Alloc::allocate(__n * sizeof(_Tp)))
                    : 0;
  }

  // __p is not permitted to be a null pointer.
  void deallocate(pointer __p, size_type __n)
  {
    _Alloc::deallocate(__p, __n * sizeof(_Tp));
  }

  size_type max_size() const __STL_NOTHROW
  {
    return size_t(-1) / sizeof(_Tp);
  }

  void construct(pointer __p, const _Tp &__val) { new (__p) _Tp(__val); }
  void destroy(pointer __p) { __p->~_Tp(); }
};

template <>
class allocator<void>
{
public:
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef void *pointer;
  typedef const void *const_pointer;
  typedef void value_type;

  template <class _Tp1>
  struct rebind
  {
    typedef allocator<_Tp1> other;
  };
};

template <class _T1, class _T2>
inline bool operator==(const allocator<_T1> &, const allocator<_T2> &)
{
  return true;
}

template <class _T1, class _T2>
inline bool operator!=(const allocator<_T1> &, const allocator<_T2> &)
{
  return false;
}

// Allocator adaptor to turn an SGI-style allocator (e.g. alloc, malloc_alloc)
// into a standard-conforming allocator.   Note that this adaptor does
// *not* assume that all objects of the underlying alloc class are
// identical, nor does it assume that all of the underlying alloc's
// member functions are static member functions.  Note, also, that
// __allocator<_Tp, alloc> is essentially the same thing as allocator<_Tp>.

template <class _Tp, class _Alloc>
struct __allocator
{
  _Alloc __underlying_alloc;

  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef _Tp *pointer;
  typedef const _Tp *const_pointer;
  typedef _Tp &reference;
  typedef const _Tp &const_reference;
  typedef _Tp value_type;

  template <class _Tp1>
  struct rebind
  {
    typedef __allocator<_Tp1, _Alloc> other;
  };

  __allocator() __STL_NOTHROW {}
  __allocator(const __allocator &__a) __STL_NOTHROW
      : __underlying_alloc(__a.__underlying_alloc) {}
  template <class _Tp1>
  __allocator(const __allocator<_Tp1, _Alloc> &__a) __STL_NOTHROW
      : __underlying_alloc(__a.__underlying_alloc) {}
  ~__allocator() __STL_NOTHROW {}

  pointer address(reference __x) const { return &__x; }
  const_pointer address(const_reference __x) const { return &__x; }

  // __n is permitted to be 0.
  _Tp *allocate(size_type __n, const void * = 0)
  {
    return __n != 0
               ? static_cast<_Tp *>(__underlying_alloc.allocate(__n * sizeof(_Tp)))
               : 0;
  }

  // __p is not permitted to be a null pointer.
  void deallocate(pointer __p, size_type __n)
  {
    __underlying_alloc.deallocate(__p, __n * sizeof(_Tp));
  }

  size_type max_size() const __STL_NOTHROW
  {
    return size_t(-1) / sizeof(_Tp);
  }

  void construct(pointer __p, const _Tp &__val) { new (__p) _Tp(__val); }
  void destroy(pointer __p) { __p->~_Tp(); }
};

template <class _Alloc>
class __allocator<void, _Alloc>
{
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef void *pointer;
  typedef const void *const_pointer;
  typedef void value_type;

  template <class _Tp1>
  struct rebind
  {
    typedef __allocator<_Tp1, _Alloc> other;
  };
};

template <class _Tp, class _Alloc>
inline bool operator==(const __allocator<_Tp, _Alloc> &__a1,
                       const __allocator<_Tp, _Alloc> &__a2)
{
  return __a1.__underlying_alloc == __a2.__underlying_alloc;
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
template <class _Tp, class _Alloc>
inline bool operator!=(const __allocator<_Tp, _Alloc> &__a1,
                       const __allocator<_Tp, _Alloc> &__a2)
{
  return __a1.__underlying_alloc != __a2.__underlying_alloc;
}
#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */

// Comparison operators for all of the predifined SGI-style allocators.
// This ensures that __allocator<malloc_alloc> (for example) will
// work correctly.

template <int inst>
inline bool operator==(const __malloc_alloc_template<inst> &,
                       const __malloc_alloc_template<inst> &)
{
  return true;
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
template <int __inst>
inline bool operator!=(const __malloc_alloc_template<__inst> &,
                       const __malloc_alloc_template<__inst> &)
{
  return false;
}
#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */

template <class _Alloc>
inline bool operator==(const debug_alloc<_Alloc> &,
                       const debug_alloc<_Alloc> &)
{
  return true;
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
template <class _Alloc>
inline bool operator!=(const debug_alloc<_Alloc> &,
                       const debug_alloc<_Alloc> &)
{
  return false;
}
#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */

// Another allocator adaptor: _Alloc_traits.  This serves two
// purposes.  First, make it possible to write containers that can use
// either SGI-style allocators or standard-conforming allocator.
// Second, provide a mechanism so that containers can query whether or
// not the allocator has distinct instances.  If not, the container
// can avoid wasting a word of memory to store an empty object.

// This adaptor uses partial specialization.  The general case of
// _Alloc_traits<_Tp, _Alloc> assumes that _Alloc is a
// standard-conforming allocator, possibly with non-equal instances
// and non-static members.  (It still behaves correctly even if _Alloc
// has static member and if all instances are equal.  Refinements
// affect performance, not correctness.)

// There are always two members: allocator_type, which is a standard-
// conforming allocator type for allocating objects of type _Tp, and
// _S_instanceless, a static const member of type bool.  If
// _S_instanceless is true, this means that there is no difference
// between any two instances of type allocator_type.  Furthermore, if
// _S_instanceless is true, then _Alloc_traits has one additional
// member: _Alloc_type.  This type encapsulates allocation and
// deallocation of objects of type _Tp through a static interface; it
// has two member functions, whose signatures are
//    static _Tp* allocate(size_t)
//    static void deallocate(_Tp*, size_t)

// The fully general version.

template <class _Tp, class _Allocator>
struct _Alloc_traits
{
  static const bool _S_instanceless = false;
  typedef typename _Allocator::__STL_TEMPLATE rebind<_Tp>::other
      allocator_type;
};

template <class _Tp, class _Allocator>
const bool _Alloc_traits<_Tp, _Allocator>::_S_instanceless;

// The version for the default allocator.

template <class _Tp, class _Tp1>
struct _Alloc_traits<_Tp, allocator<_Tp1>>
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, alloc> _Alloc_type;
  typedef allocator<_Tp> allocator_type;
};

// Versions for the predefined SGI-style allocators.

template <class _Tp, int __inst>
struct _Alloc_traits<_Tp, __malloc_alloc_template<__inst>>
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, __malloc_alloc_template<__inst>> _Alloc_type;
  typedef __allocator<_Tp, __malloc_alloc_template<__inst>> allocator_type;
};

template <class _Tp, bool __threads, int __inst>
struct _Alloc_traits<_Tp, __default_alloc_template<__threads, __inst>>
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, __default_alloc_template<__threads, __inst>>
      _Alloc_type;
  typedef __allocator<_Tp, __default_alloc_template<__threads, __inst>>
      allocator_type;
};

template <class _Tp, class _Alloc>
struct _Alloc_traits<_Tp, debug_alloc<_Alloc>>
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, debug_alloc<_Alloc>> _Alloc_type;
  typedef __allocator<_Tp, debug_alloc<_Alloc>> allocator_type;
};

// Versions for the __allocator adaptor used with the predefined
// SGI-style allocators.

template <class _Tp, class _Tp1, int __inst>
struct _Alloc_traits<_Tp,
                     __allocator<_Tp1, __malloc_alloc_template<__inst>>>
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, __malloc_alloc_template<__inst>> _Alloc_type;
  typedef __allocator<_Tp, __malloc_alloc_template<__inst>> allocator_type;
};

template <class _Tp, class _Tp1, bool __thr, int __inst>
struct _Alloc_traits<_Tp,
                     __allocator<_Tp1,
                                 __default_alloc_template<__thr, __inst>>>
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, __default_alloc_template<__thr, __inst>>
      _Alloc_type;
  typedef __allocator<_Tp, __default_alloc_template<__thr, __inst>>
      allocator_type;
};

template <class _Tp, class _Tp1, class _Alloc>
struct _Alloc_traits<_Tp, __allocator<_Tp1, debug_alloc<_Alloc>>>
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, debug_alloc<_Alloc>> _Alloc_type;
  typedef __allocator<_Tp, debug_alloc<_Alloc>> allocator_type;
};

#endif /* __STL_USE_STD_ALLOCATORS */

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#endif

__STL_END_NAMESPACE

#undef __PRIVATE

#endif /* __SGI_STL_INTERNAL_ALLOC_H */

// Local Variables:
// mode:C++
// End:
