/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#ifndef __TCPCONNECTION_H__
#define __TCPCONNECTION_H__


#include "Callbacks.hpp"
#include "Buffer.hpp"
#include "noncopyable.hpp"
#include "InetAddress.hpp"
#include <memory>
#include <string>
#include <atomic>


class Channel;
class EventLoop;
class Socket;
/*
*
*   TcpServer => Acceptor => 有一个新用户连接 通过accept 函数 拿到connfd
*     
*   =>TcpConnection 设置 回调 => Channel => 通过 poller => 调用Channel的回调操作
*
*
*/

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{

public:
    TcpConnection(EventLoop *loop, const std::string &name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const {return peerAddr_; }

    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }

    //send message
    void send(const void * message, int len);
    //关闭连接
    void shutdown();    

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

    void setCloseCallback(const CloseCallback& cb)
    { closeCallback_ = cb; }

    //当TcpServer接受一个新连接时 应该只能被调用一次
    void connectEstablished();
    //当TcpServer 移除 断开 一个连接时， 也应该只能被调用一次
    void connectDestoryed();

private:
    enum StateE { kDisconnected, KConnecting, kConnected, kDisconnecting };
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void setState(StateE state) { state_ = state; }

    void sendInLoop( const void* message, size_t len);
    void shutdownInLoop();

    EventLoop  *loop_; //这里 不可能 是baseloop 因为 TcpConnection 都是在subloop里面管理的
    const std::string name_;
    std::atomic_int state_;
    bool reading_;

    // 跟Acceptor 类似 Acceptor => mainloop    TcpConnection => subloop
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    size_t highWaterMark_;


    Buffer inputBuffer_;
    Buffer outputBuffer_;

};


#endif
