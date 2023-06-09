﻿//
// Created by bxc on 2023/3/2.
//

#ifndef RTPSERVER_LOG_H
#define RTPSERVER_LOG_H

#include <time.h>
#include <chrono>
#include <string>

static std::string getCurTimeStr(const char* time_fmt = "%Y-%m-%d %H:%M:%S") {
    time_t t = time(nullptr);
    char time_str[64];
    strftime(time_str, sizeof(time_str), time_fmt, localtime(&t));

    return time_str;
}

//  __FILE__ 获取源文件的相对路径和名字
//  __LINE__ 获取该行代码在文件中的行号
//  __func__ 或 __FUNCTION__ 获取函数名

#define LOGI(format, ...)  fprintf(stderr,"[INFO]%s [%s:%d %s()] " format "\n", getCurTimeStr().data(),__FILE__,__LINE__,__func__ ,##__VA_ARGS__)
#define LOGE(format, ...)  fprintf(stderr,"[ERROR]%s [%s:%d %s()] " format "\n",getCurTimeStr().data(),__FILE__,__LINE__,__func__ ,##__VA_ARGS__)

#endif //RTPSERVER_LOG_H