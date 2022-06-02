/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#ifndef __BUFFER_H__
#define __BUFFER_H__

#include"logger.hpp"

//网络库底层的缓冲器类型定义



#include<vector>
#include<string>
#include<algorithm>







/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        :buffer_(kCheapPrepend + initialSize),
        readerIndex_(kCheapPrepend),
        writerIndex_(kCheapPrepend)
{

}

    // 返回  数据 相应功能长度 以下函数都是
    size_t readableBytes() const {
        return writerIndex_ - readerIndex_;
    }

    size_t writableBytes() const {
        return buffer_.size() - writerIndex_;
    }

    size_t prependableBytes() const{
        return readerIndex_;
    }

    //返回可读数据的起始地址
    const char* peek(){
        return begin() + readerIndex_;
    }

    void retrieve(size_t len){
        if(len < readableBytes()){
            readerIndex_ += len; //应用 只读了 可读缓冲区 一部分  长度 未全部读完 所以 要执行该操作
        }else{// len == readableBytes
            retrieveAll();
        }
    }

    void retrieveAll(){
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }
    // 把 onMessage 函数 上报的 Buffer 数据 转成 string 数据 返回 
    std::string retrieveAllAsString(){
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len){
        if(len <= readableBytes()){
            std::string result(peek(), len);
            //数据读完了 要把 缓冲区 复位操作
            retrieve(len);
            return result;
        }
        else{
            LOG_ERROR("reading len more than len of readable error in %s:%s:%d",
                __FILE__, __FUNCTION__, __LINE__);

            return " ";
        }
    }

    void ensureWritableBytes(size_t len){
        if(writableBytes() < len){
            makeSpace(len);
        }
    }

    void append(const char* data, size_t len){
        ensureWritableBytes(len);
        //把data起始 到data+len 的数据 copy 到 beginWrite 位置上
        std::copy(data, data+len, beginWrite());
        //数据写入完成 更新writeIndex
        writerIndex_ += len;
    }

    ssize_t readFd(int fd, int* saveError);

    ssize_t writeFd(int fd, int * saveError);
private: 
    char* begin(){
        return &*buffer_.begin();
    }

    const char* begin() const {
        return &*buffer_.begin();
    }

    char * beginWrite(){
        return begin() + writerIndex_;
    }
    const char* beginWrite() const{
        return begin() + writerIndex_;
    }
    

    //要深入 理解一下
    void makeSpace(size_t len){
        if(writableBytes() + prependableBytes() <= len + kCheapPrepend){
            buffer_.resize(writerIndex_ + len);
        }else{
            if(kCheapPrepend < readerIndex_){
                size_t readable = readableBytes();
                std::copy(begin() + readerIndex_,
                        begin() + writerIndex_,
                        begin() + kCheapPrepend);
                readerIndex_ = kCheapPrepend;
                writerIndex_ = readerIndex_ + readable;
            }
        }
    }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;



};

#endif