
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


//该函数先分配array的头部，
//然后调用ngx_array_init分配存储信息的内存块，在内存池上
ngx_array_t *
ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size)
{
    ngx_array_t *a;

    //在一个内存池上分配空间，大小是一个ngx_array_t的大小
    a = ngx_palloc(p, sizeof(ngx_array_t));
    if (a == NULL) {
        return NULL;
    }
    //对其进行初始化
    //初始化的目的在与，分配存储空间
    if (ngx_array_init(a, p, n, size) != NGX_OK) {
        return NULL;
    }

    return a;
}

//销毁array的函数
//但是该函数可能并没有想象中的作用
//他可能并不真正的释放内存
void
ngx_array_destroy(ngx_array_t *a)
{
    ngx_pool_t  *p;

    p = a->pool;

    //如果array存储信息的内存块分配在内存池中的 p->d.last位置，那么last可以直接回滚
    if ((u_char *) a->elts + a->size * a->nalloc == p->d.last) {
        p->d.last -= a->size * a->nalloc;
    }

    //同样对于array的头部
    if ((u_char *) a + sizeof(ngx_array_t) == p->d.last) {
        p->d.last = (u_char *) a;
    }
}

//判断当前容器是否还能存一个元素
//如果不能就扩充
//如果可以就存放下一个元素的指针
//然后具体存放元素，是使用ngx_memzero(指针, sizeof(ngx_listening_t));
//然后直接存。并不是真正意义上的push
void *
ngx_array_push(ngx_array_t *a)
{
    void        *elt, *new;
    size_t       size;
    ngx_pool_t  *p;

    //当前已存元素个数==分配的存储信息的内存块最大存储数量
    //也就是存满了
    if (a->nelts == a->nalloc) {

        /* the array is full */

        //计算上次分配的大小，用来计算当前块是否是在内存池中最后一块
        size = a->size * a->nalloc;

        p = a->pool;

        //这里是判断array的内存块在内存池中的位置是否是最后分配的位置
        //如果是，并且能够存放下下一次存放的大小
        //那么 p->d.last直接往下推进
        //然后array最大存储nalloc++
        //p->d.last + a->size <= p->d.end这里也就是说只分配一个元素的空间大小
        if ((u_char *) a->elts + size == p->d.last
            && p->d.last + a->size <= p->d.end)
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */

            p->d.last += a->size;
            a->nalloc++;

        } else {
            /* allocate a new array */
            //分配一个新的内存块
            //分配大小是2*size，用来存储之前的元素和将来的元素
            //相当于，容器扩充
            new = ngx_palloc(p, 2 * size);
            if (new == NULL) {
                return NULL;
            }
            //然后将之前的元素，全都拷贝到新的位置
            ngx_memcpy(new, a->elts, size);
            //指向新的位置
            a->elts = new;
            //最大可分配元素个数*2
            a->nalloc *= 2;
        }
    }

    //计算存放下一个元素位置的指针
    elt = (u_char *) a->elts + a->size * a->nelts;
    //存放元素++
    a->nelts++;
    //返回该指针
    return elt;
}


//依次存放n个，基本同上
void *
ngx_array_push_n(ngx_array_t *a, ngx_uint_t n)
{
    void        *elt, *new;
    size_t       size;
    ngx_uint_t   nalloc;
    ngx_pool_t  *p;

    size = n * a->size;

    if (a->nelts + n > a->nalloc) {

        /* the array is full */

        p = a->pool;

        if ((u_char *) a->elts + a->size * a->nalloc == p->d.last
            && p->d.last + size <= p->d.end)
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */

            p->d.last += size;
            a->nalloc += n;

        } else {
            /* allocate a new array */

            nalloc = 2 * ((n >= a->nalloc) ? n : a->nalloc);

            new = ngx_palloc(p, nalloc * a->size);
            if (new == NULL) {
                return NULL;
            }

            ngx_memcpy(new, a->elts, a->nelts * a->size);
            a->elts = new;
            a->nalloc = nalloc;
        }
    }

    elt = (u_char *) a->elts + a->size * a->nelts;
    a->nelts += n;

    return elt;
}
