/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#include "TcpConnection.hpp"
#include "Callbacks.hpp"
#include "logger.hpp"
#include "Socket.hpp"
#include "Channel.hpp"
#include "EventLoop.hpp"

#include <functional>
#include <errno.h>


static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d TcpConnection loop is null", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr)
            :loop_(CheckLoopNotNull(loop)),
            name_(name),
            state_(KConnecting),
            reading_(true),
            socket_(new Socket(sockfd)),
            channel_(new Channel(loop, sockfd)),
            localAddr_(localAddr),
            peerAddr_(peerAddr),
            highWaterMark_(64*1024*1024){//64M


    // 给channel 设置了回调  poller给channel通知感兴趣的事件发生了 channel 会 回调相应操作
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this, std::placeholders::_1)
    );

    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this)
    );

    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this)
    );

    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError, this)
    );

    LOG_INFO("TcpConnection::ctor[%s] at fd = %d\n", name_.c_str(), sockfd);

    socket_->setKeepAlive(true);

}
TcpConnection::~TcpConnection(){

    LOG_INFO("TcpConnection::ctor[%s] at fd = %d  state = %d \n", name_.c_str(), channel_->fd(), static_cast<int>(state_) );

}

void TcpConnection::handleRead(Timestamp receiveTime){
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0){
        //获取 当前TcpConnection 的一个智能指针 类声明中 加了public std::enable_shared_from_this<TcpConnection>
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }else if (n == 0){
        //客户端断开连接
        handleClose();
    }else{
        //error
        errno = savedErrno;
        LOG_ERROR("TcpConnection::handleRead ");
        handleError();
    }
}

void TcpConnection::handleWrite(){
    if (channel_->isWriting()){
        int saveErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saveErrno);
        if (n > 0){
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0){
                channel_->disableWriting();
                if(writeCompleteCallback_){
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if(state_ == kDisconnecting){
                    shutdownInLoop();
                }
            }
        }else{
            //error
            LOG_ERROR("TcpConnection::handleWrite");
        }
    }else{
        LOG_ERROR("TcpConnection fd = %d is down , no more writing", channel_->fd());
    }
}

void TcpConnection::handleClose(){
    LOG_INFO("fd = %d  state = %d ", channel_->fd(), static_cast<int>(state_));
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr); //执行连接关闭的回调

    closeCallback_(connPtr);//关闭连接的回调
}

void TcpConnection::handleError(){
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if ( getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0){
        err = errno;
    }else{
        err = optval;
    }

    LOG_ERROR("TcpConnection::handleError name : %s  - SO_ERROR: %d \n", name_.c_str(), err);
}

