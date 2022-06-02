/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#ifndef __EPOLL_POLLER_H__
#define __EPOLL_POLLER_H__

#include<vector>

#include"Poller.hpp"



struct epoll_event;

class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop* loop);
    ~EPollPoller() override;  

    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;

    void updateChannel(Channel* channel) override;

    void removeChannel(Channel* channel) override;



private:
    static const int kInitEventListSize = 16;

    static const char* operationToString(int op);

    void fillActiveChannels(int numEvents, ChannelList* activeChannels)const;

    void update(int operation, Channel* channel);

    using EventList = std::vector<epoll_event>;

    int epollfd_;

    EventList events_;
};


#endif