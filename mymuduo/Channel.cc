/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#include "Channel.hpp"

#include<sstream>
//#include<cassert>


#include<sys/epoll.h>
#include"EventLoop.hpp"

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

//EventLoop: ChannelList Poller
Channel::Channel(EventLoop* loop, int fd)
    :loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1),
    tied_(false),
    eventHandling_(false),
    addedToLoop_(false),
    logHup_(true){
}

Channel::~Channel()
{
    //static_assert(!eventHandling_,"dd");
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    //强弱智能指针的应用 
    tied_ = true;

}

//作用: 当改变channel 所表示fd的events事件后，update负责在
//poller里面改变fd相应的事件epoll_ctl
//ChannelList 、 Poller(俩孩子) 都包含在 EventLoop(爹)里
//去epoll里更新    
void Channel::update()
{
    //通过channel所属的EventLoop,调用polller的相应方法，注册fd的events事件
    //add code 
    //loop_->updateChannel(this);
    //访问poller里的方法 操作事件
    //去epoll里 添加 删除事件
}
//
//在channel所属的EventLoop中，把当前的channel删除掉
void Channel::remove()
{
    //loop_->removeChannel(this);
}


void Channel::handleEvent(Timestamp receiveTime)
{
    
    if(tied_){
        std::shared_ptr<void> guard = tie_.lock();
        if(guard){
            handleEventWithGuard(receiveTime);
        }
    }else{
        handleEventWithGuard(receiveTime);
    }
}


//根据poller通知的channel发生的具体事件，由channel负责调用具体的回调操作
void Channel::handleEventWithGuard (Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revents: %d\n",revents_);
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)){
        if(closeCallback_){
            closeCallback_();
        }
    }

    if(revents_ & EPOLLERR){
        if(errorCallback_){
            errorCallback_();
        }
    }

    if(revents_ & (EPOLLIN | EPOLLPRI)){
        if(readCallback_){
            readCallback_(receiveTime);
        }
    }

    if(revents_ & (EPOLLOUT)){
        if(writeCallback_){
            writeCallback_();
        }
    }
}