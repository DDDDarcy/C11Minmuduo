/*
*
*   Copyright (c) 2022.4, ddddarcy
*
*   reconstruct simple muduo lib using C11
*
*/
#include"logger.hpp"
#include <iostream>
#include"Timestamp.hpp"

//获取唯一日志
Logger& Logger::instance(){
    static Logger logger;
    return logger;
}

//设置日志级别
void Logger::setLogLevel(int level){
    logLevel_ = level;
}

//写日志   [级别信息] time : msg
void Logger::log(std::string msg){
    switch  (logLevel_){
        case INFO: 
            std::cout<<"[INFO]";
            break;
        case ERROR: 
            std::cout<<"[ERROR]";
            break;
        case FATAL: 
            std::cout<<"[FATAL]";
            break;
        case DEBUG: 
            std::cout<<"[DEBUG]";
            break;            
        default:
            break;
    }

    //打印时间
    std::cout<<Timestamp::now().toString()<<" : "<< msg << std::endl;
}