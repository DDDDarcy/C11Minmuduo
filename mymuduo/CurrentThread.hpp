/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#ifndef __CURRENT_THREAD_H__
#define __CURRENT_THREAD_H__

#include <sys/syscall.h>
#include <unistd.h>


namespace CurrentThread
{
    extern __thread int t_cachedTid; // __thread 是 local thread 的意思

    void cacheTid();

    inline int tid()
    {
        if(__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}



#endif