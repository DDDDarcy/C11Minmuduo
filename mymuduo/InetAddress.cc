/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#include"InetAddress.hpp"
#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>



InetAddress::InetAddress(uint16_t port, std::string ip){
    bzero(&addr_,sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    addr_.sin_port = htons(port);

}


std::string InetAddress::toIp() const{
    char buf[64] = {0}; 
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    return buf;
}

std::string InetAddress::toIpPort() const{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    size_t end = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf+end,":%u" , port);
    return buf;
}

uint16_t InetAddress::port() const{
    return ntohs(addr_.sin_port);
}

#include<iostream>
using namespace std;

int main(){
    InetAddress addr(8080);
    std::cout<< addr.toIpPort()<<std::endl;

    ushort a = 8080;
    uint32_t s = a;
    
    char t = static_cast<char>(s);
    ushort b = s;
    cout<<b<<endl;
}