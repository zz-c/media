//
// Created by bxc on 2023/3/2.
//

#ifndef RTPSERVER_UTILS_H
#define RTPSERVER_UTILS_H

#include <time.h>
#include <chrono>
#include <string>

static int64_t getCurTime()// 获取当前系统启动以来的毫秒数
{
    long long now = std::chrono::steady_clock::now().time_since_epoch().count();
    return now / 1000000;
}
static int64_t getCurMillisecond()// 获取毫秒级时间戳（13位）
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).
        count();

}

/**
 * 获取1970年至今的毫秒数
 * @param system_time 是否为系统时间(系统时间可以回退),否则为程序启动时间(不可回退)
 *
 */
static int64_t getCurrentMillisecond(bool system_time = false) {

    return getCurMillisecond();
}


#endif //RTPSERVER_UTILS_H