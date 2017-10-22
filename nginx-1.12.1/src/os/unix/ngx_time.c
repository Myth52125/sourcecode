
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * FreeBSD does not test /etc/localtime change, however, we can workaround it
 * by calling tzset() with TZ and then without TZ to update timezone.
 * The trick should work since FreeBSD 2.1.0.
 *
 * Linux does not test /etc/localtime change in localtime(),
 * but may stat("/etc/localtime") several times in every strftime(),
 * therefore we use it to update timezone.
 *
 * Solaris does not test /etc/TIMEZONE change too and no workaround available.
 */

void
ngx_timezone_update(void)
{
#if (NGX_FREEBSD)
    //如果设置了时区，直接返回
    if (getenv("TZ")) {
        return;
    }

    //如果没有设置时区
    //那么先设置为utc,然后清空，然后再使用tzset()重新加载
    //http://blog.csdn.net/xin_yu_xin/article/details/38371837
    putenv("TZ=UTC");

    //tzset完成的工作是把当前时区信息（通过TZ环境变量或者/etc/localtime）读入并缓冲,重新加载
    tzset();
    //删除定义，不存在也不出错
    unsetenv("TZ");
    //加载系统默认的配置
    tzset(); 
    //如果没有指定TZ环境变量，那么缺省的时区配置文件可以用/etc/localtime来获得
    //这个文件可能是一个符号链接指向真实的文件，也有可能就是将/usr/share/zoneinfo下的文件复制过来达到所要的结果。
    //由于环境变量由各个进程组单独继承，那么在设置时区之后很难改变其他进程组的环境变量
    //所以一般的系统很少直接设置TZ环境变量，而是由/etc/localtime文件来指示时区位置
#elif (NGX_LINUX)
    time_t      s;
    struct tm  *t;
    char        buf[4];

    s = time(0); 
    //获取当前日期并转化为本地时间
    t = localtime(&s);
    //格式化时间
    strftime(buf, 4, "%H", t);

#endif
}


void
ngx_localtime(time_t s, ngx_tm_t *tm)
{
#if (NGX_HAVE_LOCALTIME_R)
    (void) localtime_r(&s, tm);

#else
    ngx_tm_t  *t;

    t = localtime(&s);
    *tm = *t;

#endif

    tm->ngx_tm_mon++;
    tm->ngx_tm_year += 1900;
}


void
ngx_libc_localtime(time_t s, struct tm *tm)
{
#if (NGX_HAVE_LOCALTIME_R)
    (void) localtime_r(&s, tm);

#else
    struct tm  *t;

    t = localtime(&s);
    *tm = *t;

#endif
}


void
ngx_libc_gmtime(time_t s, struct tm *tm)
{
#if (NGX_HAVE_LOCALTIME_R)
    (void) gmtime_r(&s, tm);

#else
    struct tm  *t;

    t = gmtime(&s);
    *tm = *t;

#endif
}
