/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#include "EventLoopThread.hpp"
#include "EventLoop.hpp"
#include <memory>


EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, 
        const std::string& name)
        :loop_(nullptr)
        ,exiting_(false)
        ,thread_(std::bind(&EventLoopThread::threadFunc, this), name)
        ,mutex_()
        ,cond_()
        ,callback_(cb)
{

}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if ( loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}


EventLoop* EventLoopThread::startLoop()
{
    thread_.start();//启动低层新线程

    EventLoop * loop = nullptr;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        while( loop_ == nullptr )
        {
            cond_.wait(lock);
        }//wait 返回 接收到来自 threadFunc的cond_.notify_one(); 说明 loop_ 已经分配好了

        loop = loop_;
    }

    return loop;

}


void EventLoopThread::threadFunc()
{   
    EventLoop loop;//创建一个独立的eventloop 和上面的线程一一对应， one loop per thread

    if(callback_)
    {   
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one(); //谁来接收呢？    EventLoopThread::startLoop() 来接收
    }

    loop.loop();

    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;

}