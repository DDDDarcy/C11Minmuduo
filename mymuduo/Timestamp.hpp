/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

#include <string>
#include<iostream>


class Timestamp{

public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);
    static Timestamp now();
    std::string toString() const;
private:
    int64_t microSecondsSinceEpoch_;
};    


#endif