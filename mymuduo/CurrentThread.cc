/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#include"CurrentThread.hpp"


namespace CurrentThread
{
    __thread int t_cachedTid = 0;

    void cacheTid()
    {
        if(t_cachedTid == 0)
        {
            //通过linux 系统调用获取线程ID  tid值
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }

}