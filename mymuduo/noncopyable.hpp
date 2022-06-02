/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#ifndef __NONCOPYABLE_H__
#define __NONCOPYABLE_H__


class noncopyable{

public: 
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
   // noncopyable(int a){int c= a;}
protected:
    noncopyable() = default;
    ~noncopyable() = default;

};

#endif