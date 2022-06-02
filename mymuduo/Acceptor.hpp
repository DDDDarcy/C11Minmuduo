/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/

#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include<functional>

#include"Channel.hpp"
#include"Socket.hpp"

class EventLoop;
class InetAddress;

class Acceptor : noncopyable
{
public:
    using NewConnectionCallBack = std::function<void (int sockfd, const InetAddress&)>;
    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallBack(const NewConnectionCallBack& cb) 
    { NewConnectionCallBack_ = cb;}

    void listen();

    bool listenning() const { return listening_; }

private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallBack NewConnectionCallBack_;
    bool listening_;
    int idleFd_;

};





#endif