/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/


#include"Acceptor.hpp"
#include"EventLoop.hpp"
#include"logger.hpp"
#include"InetAddress.hpp"

#include<errno.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>

static int createNonblocking(){
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0){
        LOG_ERROR("%s : %s : %d listen socket create err: %d\n",__FILE__, __FUNCTION__, __LINE__, errno);
    }
    return sockfd;
}


Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : loop_(loop)
    , acceptSocket_(createNonblocking())
    , acceptChannel_(loop, acceptSocket_.fd())
    , listening_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr);
    // TcpServer::start() 开始 Acceptor.listen 监听到 
    //有新用户连接 要执行一个回调 将connfd打包--》channel 交给--》subloop处理 connfd新连接的 事件
    //baseloop(或者mainloop) --》acceptChannel_(listenfd)-->
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));

}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen() 
{
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

//listenfd 发生新用户连接要执行这个回调了
void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if( connfd >= 0)
    {
        if(NewConnectionCallBack_)
        {
            NewConnectionCallBack_(connfd, peerAddr);
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s : %s : %d accept err: %d\n",__FILE__, __FUNCTION__, __LINE__, errno);
        if(errno == EMFILE)
        {
            LOG_ERROR("%s : %s : %d sockfd reached limit: %d\n",__FILE__, __FUNCTION__, __LINE__, errno);
        }
    }
}
