/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#ifndef __EVENTLOOPTHREAD_H__
#define __EVENTLOOPTHREAD_H__

#include<functional>
#include<string>
#include<mutex>
#include<condition_variable>


#include"Thread.hpp"
#include "noncopyable.hpp"


class EventLoop;


class EventLoopThread : noncopyable
{
public: 
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    /**
    * @param cb   EventLoopThread callback functional object
    * @param name thread's name
    */
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    const std::string& name = std::string());

    ~EventLoopThread();
    EventLoop* startLoop();
private:
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;



};


#endif