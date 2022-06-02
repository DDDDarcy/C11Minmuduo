/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
//扩展Poller 的派生类用

#include"Poller.hpp"
#include "EPollPoller.hpp"



Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    if(::getenv("MUDUO_USE_POLL")){
        return nullptr;
    }else{
        return new EPollPoller(loop);
    }
}