/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/

#ifndef __CHANNEL_H__
#define __CHANNEL_H__


#include"noncopyable.hpp"
#include"Timestamp.hpp"
#include"logger.hpp"

#include<functional>
#include<memory>

//前置声明 防止多包含头文件 ，将头文件放在cpp cc源文件中 
class EventLoop;
//class Timestamp;

//class 默认private 继承方式
//Channel 理解为通道，封装了sockfd和其感兴趣的event 如EPOLLIN EPOLLOUT
//还绑定了poller返回的具体事件
class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    //这里需要实例化一个Timestamp 形参对象 
    //所以 无法使用 前置声明方式
    //fd得到poller通知以后，处理事件的
    void handleEvent(Timestamp receiveTime);

    //设置回调函数对象 
    void setReadCallback(ReadEventCallback cb)
    { readCallback_ = std::move(cb); }

    void setWriteCallback(EventCallback cb)
    { writeCallback_ = std::move(cb);}

    void setCloseCallback(EventCallback cb)
    { closeCallback_ = std::move(cb); }

    void setErrorCallback(EventCallback cb)
    { errorCallback_ = std::move(cb); }

    //防止当channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt;}
    

    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() {events_ &= kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    //one loop per thread
    EventLoop* ownerLoop() { return loop_; }
    void remove();



private:
    //static std::string eventsToString(int fd, int ev);

    void update();
    void handleEventWithGuard(Timestamp receiveTime);


    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;       //事件循环
    const int   fd_;        //fd ,Poller监听对象
    int         events_;    //注册fd感兴趣的事件
    int         revents_;   //poller返回的具体发生的事件
    int         index_;
    bool        logHup_;

    std::weak_ptr<void> tie_;
    bool tied_;
    bool eventHandling_;
    bool addedToLoop_;

    //因为channel通道里面能够获取fd最终发生的具体的事件revents
    //所以他负责调用具体的事件的回调操作
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

};

#endif