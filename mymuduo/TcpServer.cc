/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#include "TcpServer.hpp"
#include "EventLoop.hpp"
#include "logger.hpp"


static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainloop is null", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop* loop, 
    const InetAddress& listenAddr, 
    const std::string& nameArg,
    Option option)
    :loop_(CheckLoopNotNull(loop)),
    ipPort_(listenAddr.toIpPort()),
    name_(nameArg),
    acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
    threadpool_(new EventLoopThreadPool(loop, nameArg)),
    connectionCallback_(),
    messageCallback_(),
    nextConnId_(1)
{
    acceptor_->setNewConnectionCallBack(std::bind(&TcpServer::newConnection, this,
                std::placeholders::_1, std::placeholders::_2));
}

void TcpServer::start()
{

}

void TcpServer::setThreadNum( int numThreads)
{

}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{

}
    //Thread safe
void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{

}
    //Not thread safe, but in loop
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    
}