/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#include"Thread.hpp"
#include <cstdio>
#include <memory>
#include <semaphore.h>
#include"CurrentThread.hpp"

std::atomic_int32_t numCreated_ (0);

Thread::Thread(ThreadFunc func, const std::string &name)
    :started_(false)
    ,joined_(false)
    ,tid_(0)
    ,func_(std::move(func))
    ,name_(name)
{
    setDefaultName();
}



Thread::~Thread()
{
    if(started_ && !joined_){
        thread_->detach(); //线程分离
    }
}

void Thread::start()
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    //开启线程
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){

        tid_ = CurrentThread::tid();
        sem_post(&sem);


        func_(); //开启一个新线程 专门执行
    }));

    //必须等待 创建新线程的 tid值
    sem_wait(&sem);

    /*
    started_ = true;
    sem_t sem1;
    sem_init(&sem1, false, 0);

    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        tid_ = CurrentThread::tid();
        sem_post(&sem1);


        func();
    }));

    sem_wait(&sem);
    */

}



void Thread::join()
{
    joined_ = true;
    thread_->join();  
}


void Thread::setDefaultName()
{
    int num = numCreated_++;
    if(name_.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d ", num);
        name_ = buf;
    }
}