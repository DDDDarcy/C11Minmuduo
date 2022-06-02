/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__

#include<functional>
#include <sched.h>
#include<vector>
#include<atomic>
#include<memory>
#include<mutex>
#include<unistd.h>



#include"Timestamp.hpp"
#include"CurrentThread.hpp"


class Poller;
class Channel;

//主要包含俩模块  Channel Poller(epoll)
class EventLoop{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    //开启事件循环
    void loop();
    //退出事件循环
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_;}

    //在当前loop中执行cb 
    void runInLoop(Functor cb);
    //把cb放入队列中，唤醒loop所在的线程，执行cb
    void queueInLoop(Functor cb);

    //用来唤醒loop所在线程
    void wakeup();

    //EventLoop的方法 => Poller的方法 
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    
    bool hasChannel(Channel* channel);

    //判断EventLoop对象 是否在自己的线程里面
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
    using ChannelList = std::vector<Channel*>;

    void handleRead();//wakeup
    void doPendingFunctors();//执行回调

    std::atomic_bool looping_; // 原子操作 通过CAS实现
    std::atomic_bool quit_;     //标识 退出 loop 循环
    std::atomic_bool eventHanding_; 
    
    

    const pid_t threadId_; //记录当前loop 所在线程
    Timestamp pollReturnTime_; //poller 返回发生事件的channels的时间点
    std::unique_ptr<Poller> poller_;

    int wakeupFd_;// mainloop 获取一个新用户的 subloop时 通过轮询算法选择一个subloop，通过该成员唤醒subloop的处理
    std::unique_ptr<Channel> WakeupChannel_;

    ChannelList activeChannels_;
    Channel* currentActiveChannel_;

    std::mutex mutex_;// 互斥锁 用来 保护下面 vector 容器 线程安全操作
    std::atomic_bool callingPendingFunctors_; //标识当前loop 是否有回调操作
    std::vector<Functor> pendingFunctors_;
};

#endif