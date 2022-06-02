/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#include "EventLoop.hpp"
#include <mutex>
#include<sys/eventfd.h>
#include<fcntl.h>
#include<errno.h>
#include <unistd.h>

#include "CurrentThread.hpp"
#include "logger.hpp"
#include"Poller.hpp"
#include"Channel.hpp"


//当前线程的eventloop
__thread EventLoop* t_loopInThisThread = nullptr;

//定义默认的Poller IO 复用接口的超时时间
const int kPollTimeMs = 10000;

//创建wakeupfd, 用来notify唤醒subReactor处理新来的channel    mainloop  唤醒 subloop
int createEventfd()
{
    int evtfd = ::eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0){
        LOG_FATAL("eventfd error:%d \n",errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    :looping_(false)
    ,quit_(false)
    ,callingPendingFunctors_(false)
    ,threadId_(CurrentThread::tid())
    ,poller_(Poller::newDefaultPoller(this))
    ,wakeupFd_(createEventfd())
    ,WakeupChannel_(new Channel(this,wakeupFd_))
    ,currentActiveChannel_(nullptr)
{
    LOG_DEBUG("EventLoop create %p in thread %d \n", this, threadId_);

    if(t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread %d \n", t_loopInThisThread, threadId_);
    }else{
        t_loopInThisThread = this;
    }

    //设置wakeupfd的事件类型以及发生事件后的会掉操作 就是唤醒subloop
    WakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead,this));

    WakeupChannel_->enableReading();

}



EventLoop::~EventLoop()
{
    WakeupChannel_->disableAll();
    WakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}


void EventLoop::handleRead ()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if(n != sizeof one) 
    {
        LOG_ERROR("EventLoop::handleRead() reads %d bytes instead of 8", static_cast<int>(n));
    }

}

void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    LOG_INFO("EventLoop %p start looping ", this);

    while(!quit_){
        activeChannels_.clear();
        //监听两类 fd 一类是 client  一类是 wakeupfd
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);

        for(Channel* channel : activeChannels_){
            //Poller 监听了哪些channel发生事件了 ， 然后上报给EventLoop, 通知channel处理相应的事件
            channel->handleEvent(pollReturnTime_);

            //
            //
        }
        /*
        *  IO 线程 只有mainloop 负责 accept 把 fd 打包成 channel
        *   mainloop 事先注册一个 回调cb (需要subloop来执行) wakeup
        *   
        */

        doPendingFunctors();

    }

    LOG_INFO("EventLoop %p stop looping", this);
    looping_ = false;


}


/*
*
*                               mainloop
*
*               =========================== 搞一个生产者 消费者 的线程安全队列                       
*
*   subloop               subloop                     subloop
*
*
*
*/
void EventLoop::quit()
{
    //退出事件循环  loop在自己的线程中调用quit    或者  在非loop线程中 调用loop的quit
    quit_ = true;
    //当 subloop 修改了 mainloop的quit_  但是 不是一个线程的 所以要唤醒mainloop所在线程  在loop函数里判断quit_ 是否要poll
    if(!isInLoopThread()){ 
        wakeup();
    }

}

void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread()){
        cb();
    }else{
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        //std::lock_guard<std::mutex> lock(mutex_);
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }
    //唤醒 loop 所在线程
    // ||  callingPendingFunctors_ 是当 doPendingFunctors() 还在执行回调的时候 ，poller->poll(....) 阻塞了  唤醒它
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one){
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 \n",n);
    }
}

//EventLoop的方法 => Poller的方法   
//poller 跟 channel 通过 evenloop沟通
void EventLoop::updateChannel(Channel* channel)
{

    poller_->updateChannel(channel);

}
void EventLoop::removeChannel(Channel* channel)
{

    poller_->removeChannel(channel);
}

    
bool EventLoop::hasChannel(Channel* channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors ()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    //定义临时容器 防止 与 queueInLoop 往pending里添加
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(const Functor& functor : functors){
        functor();
    }

    callingPendingFunctors_ = false;
}

