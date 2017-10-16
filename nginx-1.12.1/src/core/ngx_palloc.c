
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


static ngx_inline void *ngx_palloc_small(ngx_pool_t *pool, size_t size,
    ngx_uint_t align);
static void *ngx_palloc_block(ngx_pool_t *pool, size_t size);
static void *ngx_palloc_large(ngx_pool_t *pool, size_t size);

//创建一块内存池
ngx_pool_t *
ngx_create_pool(size_t size, ngx_log_t *log)
{
    ngx_pool_t  *p;

    //调用posix_memalign()分配一个以NGX_POOL_ALIGNMENT对齐的内存。
    //http://www.jianshu.com/p/2f28226f5f9a
    //NGX_POOL_ALIGNMENT 16
    p = ngx_memalign(NGX_POOL_ALIGNMENT, size, log);
    if (p == NULL) {
        return NULL;
    }

    //初始化p的值
    //一个p结构的结尾，用来标记着一块内存的，下一字节为可用存储部分
    p->d.last = (u_char *) p + sizeof(ngx_pool_t);

    //整一块分配内存的结尾
    p->d.end = (u_char *) p + size;
    //新建一个内存池，只有当前着一块内存，并没有其他的块，所以设置为NULL
    p->d.next = NULL;
    //失败的次数设置为0
    p->d.failed = 0;

    //可用的存储数据部分大小
    size = size - sizeof(ngx_pool_t);
    //设置内存块的最大可用存储大小
    //NGX_MAX_ALLOC_FROM_POOL = ngx_pagesize 应该是一页的大小
    p->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL;

    //当前
    //其他部分都设置为0
    p->current = p;
    p->chain = NULL;
    //还没有分配large块
    p->large = NULL;
    //释放该内存块的结构
    p->cleanup = NULL;
    //记录？
    p->log = log;

    return p;
}


void
ngx_destroy_pool(ngx_pool_t *pool)
{
    ngx_pool_t          *p, *n;
    ngx_pool_large_t    *l;
    ngx_pool_cleanup_t  *c;

    //释放的函数，内存块以链表形式存储，调用释放函数，迭代释放
    for (c = pool->cleanup; c; c = c->next) {
        if (c->handler) {
            ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
                           "run cleanup: %p", c);
            c->handler(c->data);
        }
    }

#if (NGX_DEBUG)

    /*
     * we could allocate the pool->log from this pool
     * so we cannot use this log while free()ing the pool
     */

    for (l = pool->large; l; l = l->next) {
        ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0, "free: %p", l->alloc);
    }

    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        ngx_log_debug2(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
                       "free: %p, unused: %uz", p, p->d.end - p->d.last);

        if (n == NULL) {
            break;
        }
    }

#endif

    //这是在释放什么？
    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            ngx_free(l->alloc);
        }
    }
    //也是在释放
    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        ngx_free(p);

        if (n == NULL) {
            break;
        }
    }
}

//将一个内存块重置，先释放了
void
ngx_reset_pool(ngx_pool_t *pool)
{
    ngx_pool_t        *p;
    ngx_pool_large_t  *l;

    //释放所有的挂的large块
    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            ngx_free(l->alloc);
        }
    }

    //重置所有小的块
    for (p = pool; p; p = p->d.next) {
        p->d.last = (u_char *) p + sizeof(ngx_pool_t);
        p->d.failed = 0;
    }

    pool->current = pool;
    pool->chain = NULL;
    pool->large = NULL;
}

//分配，直接分配大块
void *
ngx_palloc(ngx_pool_t *pool, size_t size)
{
#if !(NGX_DEBUG_PALLOC)
    if (size <= pool->max) {
        return ngx_palloc_small(pool, size, 1);
    }
#endif
    return ngx_palloc_large(pool, size);
}

//就算不对齐的也要去对齐
void *
ngx_pnalloc(ngx_pool_t *pool, size_t size)
{
#if !(NGX_DEBUG_PALLOC)
    if (size <= pool->max) {
        return ngx_palloc_small(pool, size, 0);
    }
#endif

    return ngx_palloc_large(pool, size);
}

/*
#define ngx_align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))
*/
//作用是在内存池中寻找一个size的大小的内存区域，返回指针
//如果找不到，那么重新分配一块内存块，返回新的内存块
static ngx_inline void *
ngx_palloc_small(ngx_pool_t *pool, size_t size, ngx_uint_t align)
{
    u_char      *m;
    ngx_pool_t  *p;

    p = pool->current;

    do {
        //指向内存池头部的尾部，也就是没有数据的位置
        //m后面的为值都是内存池可以存储信息的位置
        m = p->d.last;

        //对m进行内存对齐，也就是对将要写数据的区域进行内存对齐
        //用来加快cpu读取内存
        //align是1或是0
        if (align) {
            //NGX_ALIGNMENT = sizeof(unsigned long) 也就是说和平台有关
            m = ngx_align_ptr(m, NGX_ALIGNMENT);
        }
       
        //(p->d.end - m) >= size剩下的空间，仍然能够放一个size
        //那么直接返回对齐的地址
        //也就是说，这个地址要用来存数据
        if ((size_t) (p->d.end - m) >= size) {
            //如果可以分配，那么更新last。
            p->d.last = m + size;
            return m;
        }

        //如果不能放下，就取下一个内存块找
        p = p->d.next;

    } while (p);

    //所有的内存块都不够大小，那么重新分配一块
    return ngx_palloc_block(pool, size);
}

//添加新的内存块
static void *
ngx_palloc_block(ngx_pool_t *pool, size_t size)
{
    //这里为什么不直接设置为ngx_pool_t类型的m，后面还要在转换
    u_char      *m;
    size_t       psize;
    ngx_pool_t  *p, *new;

    //计算当前内存块大小
    psize = (size_t) (pool->d.end - (u_char *) pool);

    // #define ngx_memalign(alignment, size, log)  ngx_alloc(size, log) 
    //ngx_alloc只是简单的封装了malloc
    //分配内存
    //NGX_POOL_ALIGNMENT =16 ，进行对齐 
    m = ngx_memalign(NGX_POOL_ALIGNMENT, psize, pool->log);
    if (m == NULL) {
        return NULL;
    }
    //—————————错了———————————
    //这里应该是写错了，应该是ngx_pool_data_t
    new = (ngx_pool_t *) m;
    //设置最后的位置
    new->d.end = m + psize;
    //没有下一块
    new->d.next = NULL;
    new->d.failed = 0;

    //再次内存对齐
    m += sizeof(ngx_pool_data_t);
    m = ngx_align_ptr(m, NGX_ALIGNMENT);

    //可写部分从ngx_pool_data_t的对齐部分开始
    new->d.last = m + size;

    //这里是重新分配可写的内存块，如果失败次数过大，那么直接跳过这块内存块
    //因为这里的d.failed++ ，因此，pool->current 会一直相会推。
    //意味着ngx_palloc_small中的while循环会少循环几次
    //因为之前不能分配，那么之后也不能分配
    for (p = pool->current; p->d.next; p = p->d.next) {
        if (p->d.failed++ > 4) {
            pool->current = p->d.next;
        }
    }
    //上面循环结束以后，p指向了内存池最后一块ngx_pool_t
    //新的内存块挂到老的上面
    p->d.next = new;

    return m;
}


static void *
ngx_palloc_large(ngx_pool_t *pool, size_t size)
{
    void              *p;
    ngx_uint_t         n;
    ngx_pool_large_t  *large;

    //分配了一个void的内存
    p = ngx_alloc(size, pool->log);
    if (p == NULL) {
        return NULL;
    }

    n = 0;

    //将用p填充最后一个的pool->large->alloc==NULL的alloc,就返回了
    for (large = pool->large; large; large = large->next) {
        if (large->alloc == NULL) {
            large->alloc = p;
            return p;
        }
        //ngx_pool_large_t链数要小于3，大于3以后执行下面的
        //也就是说，下面讲为什么
        if (n++ > 3) {
            break;
        }
    }
    //上面循环的能够结束1.找到large没有分配alloc。
    //内存池已经有3块large了

    //---下面失去新的内存块上分配ngx_pool_large_t----
    //去分配一块ngx_pool_large_t大小的内存，其大小应该只是两个指针。
    //在ngx_palloc_small中，现在已有的内存池中分配，如果没有足够的大小
    //就重新申请一块内存。
    large = ngx_palloc_small(pool, sizeof(ngx_pool_large_t), 1);
    if (large == NULL) {
        //如果分配失败，同时要释放p
        ngx_free(p);
        return NULL;
    }

    //分配成功，然后alloc挂在之前分配的内存块p
    large->alloc = p;
    //由新的large块接管以前的large链表
    large->next = pool->large;
    //pool->large指向新的large。
    //这样，新的large成为链表的头部，其他的旧链表依次相后推了。
    pool->large = large;

    //也就是说，虽然large的头部信息，可能不存在内存池的第一块内存中，但是都连接在头部信息的large中

    //新分配的large应该挂到分配ngx_pool_large_t的那个内存块上阿。
    return p;
}


//也是分配ngx_pool_large_t的函数，和上面的区别不但
//但是这个函数，默认large是内存池的第一块。
//没有判断，直接挂上了。
void *
ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment)
{
    void              *p;
    ngx_pool_large_t  *large;

    p = ngx_memalign(alignment, size, pool->log);
    if (p == NULL) {
        return NULL;
    }

    large = ngx_palloc_small(pool, sizeof(ngx_pool_large_t), 1);
    if (large == NULL) {
        ngx_free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}

//释放large的函数。
//第二个参数是什么？？？
ngx_int_t
ngx_pfree(ngx_pool_t *pool, void *p)
{
    ngx_pool_large_t  *l;

    //循环的，释放一个ngx_pool_t上的全部large
    for (l = pool->large; l; l = l->next) {
        if (p == l->alloc) {
            ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
                           "free: %p", l->alloc);
            //释放large上的内存块
            ngx_free(l->alloc);
            //将其NULL
            l->alloc = NULL;

            return NGX_OK;
        }
    }

    return NGX_DECLINED;
}


//在内存池中申请制定的大小
//但是置空了
//和malloc和calloc的关系一样
void *
ngx_pcalloc(ngx_pool_t *pool, size_t size)
{
    void *p;

    p = ngx_palloc(pool, size);
    if (p) {
        //置为0
        ngx_memzero(p, size);
    }

    return p;
}

//应该是添加一个删除器，但是没有删除函数，并且可以挂一块内存
ngx_pool_cleanup_t *
ngx_pool_cleanup_add(ngx_pool_t *p, size_t size)
{
    ngx_pool_cleanup_t  *c;

    c = ngx_palloc(p, sizeof(ngx_pool_cleanup_t));
    if (c == NULL) {
        return NULL;
    }

    //如果传入第二个参数，那么该参数代表ngx_pool_cleanup_t的data大小
    //分配data一块内存
    if (size) {
        c->data = ngx_palloc(p, size);
        if (c->data == NULL) {
            return NULL;
        }

    } else {
        //如果size=0，那么data=NULL
        c->data = NULL;
    }

    //释放函数为NULL
    c->handler = NULL;
    //——————————————
    //为什么要这样指向？？？
    c->next = p->cleanup;
    p->cleanup = c;

    ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, p->log, 0, "add cleanup: %p", c);

    return c;
}


void
ngx_pool_run_cleanup_file(ngx_pool_t *p, ngx_fd_t fd)
{
    ngx_pool_cleanup_t       *c;
    ngx_pool_cleanup_file_t  *cf;

    for (c = p->cleanup; c; c = c->next) {
        //如果ngx_pool_cleanup_add上的删除函数是指定的函数
        //那么调用函数并删除
        if (c->handler == ngx_pool_cleanup_file) {
            //这里是？？
            cf = c->data;

            if (cf->fd == fd) {
                c->handler(cf);
                c->handler = NULL;
                return;
            }
        }
    }
}

//指定的删除函数
void
ngx_pool_cleanup_file(void *data)
{
    ngx_pool_cleanup_file_t  *c = data;

    ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, c->log, 0, "file cleanup: fd:%d",
                   c->fd);

    if (ngx_close_file(c->fd) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ALERT, c->log, ngx_errno,
                      ngx_close_file_n " \"%s\" failed", c->name);
    }
}


void
ngx_pool_delete_file(void *data)
{
    ngx_pool_cleanup_file_t  *c = data;

    ngx_err_t  err;

    ngx_log_debug2(NGX_LOG_DEBUG_ALLOC, c->log, 0, "file cleanup: fd:%d %s",
                   c->fd, c->name);

    if (ngx_delete_file(c->name) == NGX_FILE_ERROR) {
        err = ngx_errno;

        if (err != NGX_ENOENT) {
            ngx_log_error(NGX_LOG_CRIT, c->log, err,
                          ngx_delete_file_n " \"%s\" failed", c->name);
        }
    }

    if (ngx_close_file(c->fd) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ALERT, c->log, ngx_errno,
                      ngx_close_file_n " \"%s\" failed", c->name);
    }
}


#if 0

static void *
ngx_get_cached_block(size_t size)
{
    void                     *p;
    ngx_cached_block_slot_t  *slot;

    if (ngx_cycle->cache == NULL) {
        return NULL;
    }

    slot = &ngx_cycle->cache[(size + ngx_pagesize - 1) / ngx_pagesize];

    slot->tries++;

    if (slot->number) {
        p = slot->block;
        slot->block = slot->block->next;
        slot->number--;
        return p;
    }

    return NULL;
}

#endif
