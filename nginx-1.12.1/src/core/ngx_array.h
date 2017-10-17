
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_ARRAY_H_INCLUDED_
#define _NGX_ARRAY_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct {
    //可存放数据的指针
    void        *elts;
    //array头部地址
    ngx_uint_t   nelts;
    //元素个数
    size_t       size;
    //每个元素的大小
    ngx_uint_t   nalloc;
    //分配内存所在的内存池
    ngx_pool_t  *pool;
} ngx_array_t; 


ngx_array_t *ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size);
void ngx_array_destroy(ngx_array_t *a);
void *ngx_array_push(ngx_array_t *a);
void *ngx_array_push_n(ngx_array_t *a, ngx_uint_t n);


//分配内存块
static ngx_inline ngx_int_t
ngx_array_init(ngx_array_t *array, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    /*
     * set "array->nelts" before "array->elts", otherwise MSVC thinks
     * that "array->nelts" may be used without having been initialized
     */
    //已存元素数量
    array->nelts = 0;
    array->size = size;
    array->nalloc = n;
    array->pool = pool;
    //具体的存储信息的部分，在内存池上分配空间。大小是n * size
    //array->elts是可存放数据的起始位置
    array->elts = ngx_palloc(pool, n * size);
    if (array->elts == NULL) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


#endif /* _NGX_ARRAY_H_INCLUDED_ */
