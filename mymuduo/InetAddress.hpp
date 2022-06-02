/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#ifndef __INETADDRESS_H__
#define __INETADDRESS_H__

#include <cstdint>
#include <string>
#include<strings.h>
#include<arpa/inet.h>
#include<netinet/in.h>




class InetAddress 
{
public:    
    explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");
    explicit InetAddress(const sockaddr_in& addr):addr_(addr){}

    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t port() const;

    
    const sockaddr_in* getSockAddr() const { return &addr_;}

    void setSockAddr(const sockaddr_in &addr) { addr_ = addr; }

private:
    sockaddr_in addr_;
};


#endif
