/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__


#include"noncopyable.hpp"
#include"Callbacks.hpp"
#include"InetAddress.hpp"
#include "Acceptor.hpp"
#include"EventLoop.hpp"
#include"EventLoopThread.hpp"
#include"EventLoopThreadPool.hpp"



#include<functional>
#include<unordered_map>
#include<string>
#include<memory>
#include<atomic>




//主要提供给外部用户使用
class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    enum Option
    {
        kNoReusePort,
        kReusePort
    };
    TcpServer(EventLoop* loop,
                const InetAddress& listenAddr,
                const std::string& nameArg,
                Option option = kNoReusePort);
    
    ~TcpServer();

    const std::string& ipPort() const { return ipPort_; }

    EventLoop* getLoop() const { return loop_; }

    //相当于 设置subloop 个数
    void setThreadNum( int numThreads );

    //thread safe
    void start();


    //the following function isn t  thread safe 
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }


private:
    //No thread safe, but in loop
    void newConnection(int sockfd, const InetAddress& peerAddr);
    //Thread safe
    void removeConnection(const TcpConnectionPtr& conn);
    //Not thread safe, but in loop
    void removeConnectionInLoop(const TcpConnectionPtr& conn);


    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;//acceptor loop
    const std::string ipPort_;
    const std::string name_;

    std::unique_ptr<Acceptor> acceptor_;    //运行在 main loop
    std::shared_ptr<EventLoopThreadPool> threadpool_; // one loop per thread

    ConnectionCallback connectionCallback_;         //有新连接的回调
    MessageCallback messageCallback_;               //接受消息的回调
    WriteCompleteCallback writeCompleteCallback_;   //消息发送完成的回调
    ThreadInitCallback threadInitCallback_;         //  loop线程初始化的回调
    std::atomic_int32_t started_;

    int nextConnId_;
    ConnectionMap connections_;                     //保存所有连接 




};

#endif