/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#ifndef __THREAD_H__
#define __THREAD_H__

#include"noncopyable.hpp"
#include<memory>
#include<functional>
#include <sched.h>
#include<thread>
#include <atomic>
#include<string>

class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc func, const std::string &name = std::string());
    ~Thread();

    void start();
    void join();

    bool started() const { return started_; }

    static int numCreated(){ return numCreated_; }

    void setDefaultName();

private:
    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    ThreadFunc func_;
    std::string  name_;
    static std::atomic_int32_t numCreated_;

};

#endif