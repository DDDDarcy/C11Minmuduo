/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#include"Timestamp.hpp"

#include<time.h>

Timestamp::Timestamp(){

}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
    :microSecondsSinceEpoch_(microSecondsSinceEpoch){

}

std::string Timestamp::toString() const{

    //定义buf
    char buf[128] = {0};
    tm * tm = localtime(&microSecondsSinceEpoch_);
    snprintf(buf,128,"%4d/%02d/%02d %02d:%02d:%02d",
        tm->tm_year + 1900,
        tm->tm_mon + 1,
        tm->tm_mday,
        tm->tm_hour,
        tm->tm_min,
        tm->tm_sec);
    return buf;
}

Timestamp Timestamp::now(){

    return Timestamp(time(NULL));
}
// #include<iostream>
// int main(){

//     std::cout << Timestamp::now().toString()<<std::endl;
//     return 0;
// }