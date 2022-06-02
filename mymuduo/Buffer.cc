/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#include"Buffer.hpp"


#include<sys/uio.h>
#include<errno.h>
#include<unistd.h>




//从fd 读数据 Poller 是LT模式
//Buffer 是有大小的 但是 从 fd上 读取数据的时候， 却不知道 tcp 数据最终大小
ssize_t Buffer::readFd(int fd, int* saveError){
    char extrabuf[65536];

    struct iovec vec[2];
    const size_t writable = writableBytes();

    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len =  writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = readv(fd, vec, iovcnt);

    if(n < 0){ //error
        *saveError = errno;
    }
    else if( n < writable ){
        writerIndex_ += n;
    }
    else{//  |------xxx--|----readbuf---|----writebuf-这里被当前填满了要去--|
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }

    return n;

}
ssize_t Buffer::writeFd(int fd, int* saveError){
    ssize_t n = ::write(fd, peek(), readableBytes());

    if (n < 0){
        *saveError = errno;
    }
    
    return n;
}