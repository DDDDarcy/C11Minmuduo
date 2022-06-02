/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#include"EPollPoller.hpp"
#include"Channel.hpp"
#include "Poller.hpp"
#include "Timestamp.hpp"
#include"logger.hpp"
#include <cstddef>
#include <cstring>



#include <sys/epoll.h>
#include<unistd.h>
#include<errno.h>


const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;



EPollPoller::EPollPoller(EventLoop* loop)
    :Poller(loop),
    epollfd_(epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize)
{
    LOG_INFO("function = %s",__FUNCTION__);

    if(epollfd_ < 0){
        LOG_FATAL("epoll create error:%d \n",errno);
    }
}

EPollPoller::~EPollPoller()
{
    LOG_INFO("function = %s",__FUNCTION__);

    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    LOG_INFO("function = %s",__FUNCTION__);

    int numEvents = ::epoll_wait(epollfd_, 
                                &*events_.begin(), 
                                static_cast<int>(events_.size()), 
                                timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    if(numEvents > 0){
        LOG_INFO("%d events happened",numEvents);
        fillActiveChannels(numEvents, activeChannels);

        if(numEvents == events_.size()){
            events_.resize(events_.size() * 2);
        }else if(numEvents == 0){
            LOG_DEBUG("%s time out!", __FUNCTION__);
        }else{
            if(saveErrno != EINTR){
                errno = saveErrno;
                LOG_ERROR("EPollPoller::poll() err!");
            }
        }
    }
    return now;
}

void EPollPoller::fillActiveChannels (int numEvents, ChannelList* activeChannels) const
{
    LOG_INFO("function = %s",__FUNCTION__);
    for(int i = 0; i < numEvents; ++i){
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->fd();//
        ChannelMap::const_iterator it = channels_.find(fd);
        if(it != channels_.end() && it->second ==channel){}
        else return;
#endif
        channel->set_revents(events_[i].events);//????? // real event 真真整整的发生的事件
        activeChannels->push_back(channel);
    }
}

void EPollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();//获取channel 的index

    LOG_INFO("function = %s => fd = %d  events = %d index = %d \n",__FUNCTION__,channel->fd(),channel->events(),index);

    if(index == kNew || index == kDeleted){
        //
        int fd = channel->fd();
        if(index == kNew){//EPOLL_CTL_ADD
            channels_[fd] = channel;
        }else{
            if(channels_.find(fd) != channels_.end() && channels_[fd] == channel){}
            else return;
        }

        //
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }else{//channel 已经在poller上注册过了
        //EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        if(channel->isNoneEvent()){
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }else{//channel 包含的fd 还对某些事情感兴趣
            update(EPOLL_CTL_MOD, channel);
        }
    }    
}


void EPollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    LOG_INFO("function = %s",__FUNCTION__);

    //1.先从map里找到这个fd的value 存在 ,不存在 return
    //2.再去匹配map里fd对应的value值是否与channel匹配，不匹配 return
    //3.真正找到这个channel确切存在于map里，再考虑channel是否还有其他读写事件需要处理，如果没有，才可以删除
    if(channels_.find(fd)==channels_.end()) return ;
    if(channels_[fd] != channel) return;
    if(channel->isNoneEvent()){
        int index = channel->index();
        if(index == kAdded || index ==kDeleted){
            size_t n = channels_.erase(fd);
            if(index == kAdded){
                update(EPOLL_CTL_DEL,channel);
            }
            channel->set_index(kNew);
        }
    }
}

void EPollPoller::update(int operation, Channel* channel)
{
    //
    epoll_event event;
    memset(&event, 0, sizeof event);

    int fd = channel->fd();


    event.events = channel->events();
    event.data.fd = fd;
    event.data.ptr = channel;

    

    if(::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }else{
            LOG_FATAL("epoll_ctl add/mod error:%d",errno);
        }
    }
}






