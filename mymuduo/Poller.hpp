/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#ifndef __POLLER_H__
#define __POLLER_H__


#include<unordered_map>
#include<vector>

#include "Timestamp.hpp"
#include "noncopyable.hpp"
#include "EventLoop.hpp"

class Channel;

class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;   

    Poller(EventLoop* loop);
    virtual ~Poller();

    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

    virtual void updateChannel(Channel* channel) = 0;

    virtual void removeChannel(Channel* channel) = 0;

    virtual bool hasChannel(Channel* channel) const;

    //新建一个DefaultPoller.hpp 文件 在其内部实现 包含派生类扩展实现 扩展性 达到 管理方便的功能
    static Poller* newDefaultPoller(EventLoop* loop);   

protected:
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop* ownerLoop_;


};


#endif