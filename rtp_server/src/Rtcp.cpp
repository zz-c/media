//
// Created by bxc on 2023/3/2.
//

#include "Rtcp.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

std::vector<RtcpHeader*> RtcpHeader::loadFromBytes(char* data, size_t len) {

    std::vector<RtcpHeader*> ret;
    ssize_t remain = len;
    char* ptr = data;
    while (remain > (ssize_t)sizeof(RtcpHeader)) {
        RtcpHeader* rtcp = (RtcpHeader*)ptr;
        auto rtcp_len = rtcp->getSize();
        if (remain < (ssize_t)rtcp_len) {
            std::cout << "非法的rtcp包,声明的长度超过实际数据长度" << std::endl;
            break;
        }
        try {
            rtcp->net2Host(rtcp_len);
            ret.emplace_back(rtcp);
        }
        catch (std::exception& ex) {
            // 不能处理的rtcp包，或者无法解析的rtcp包，忽略掉
            std::cout << ex.what() << ",长度为:" << rtcp_len << std::endl;
        }
        ptr += rtcp_len;
        remain -= rtcp_len;
    }
    return ret;
}
void RtcpHeader::net2Host(size_t len) {

    switch ((RtcpType)pt) {
    case RtcpType::RTCP_SR: {
        RtcpSR* sr = (RtcpSR*)this;
        sr->net2Host(len);
        break;
    }

    case RtcpType::RTCP_RR: {
        RtcpRR* rr = (RtcpRR*)this;
        rr->net2Host(len);
        break;
    }
    default: {
        std::cout << "未处理的rtcp包:" << this->pt << std::endl;
    }
    }
}
size_t RtcpHeader::getSize() const {
    // 加上rtcp头长度
    return (1 + ntohs(length)) << 2;
}
size_t RtcpHeader::getPaddingSize() const {
    if (!padding) {
        return 0;
    }
    return ((uint8_t*)this)[getSize() - 1];
}

void RtcpHeader::setSize(size_t size) {
    // 不包含rtcp头的长度
    length = htons((uint16_t)((size >> 2) - 1));
}

void ReportItem::net2Host() {
    ssrc = ntohl(ssrc);
    cumulative = ntohl(cumulative) >> 8;
    seq_cycles = ntohs(seq_cycles);
    seq_max = ntohs(seq_max);
    jitter = ntohl(jitter);
    last_sr_stamp = ntohl(last_sr_stamp);
    delay_since_last_sr = ntohl(delay_since_last_sr);
}


void RtcpSR::net2Host(size_t size) {
    static const size_t kMinSize = sizeof(RtcpSR) - sizeof(items);
    //CHECK_MIN_SIZE(size, kMinSize);

    ssrc = ntohl(ssrc);
    ntpmsw = ntohl(ntpmsw);
    ntplsw = ntohl(ntplsw);
    rtpts = ntohl(rtpts);
    packet_count = ntohl(packet_count);
    octet_count = ntohl(octet_count);

    ReportItem* ptr = &items;
    int item_count = 0;
    for (int i = 0; i < (int)report_count && (char*)(ptr)+sizeof(ReportItem) <= (char*)(this) + size; ++i) {
        ptr->net2Host();
        ++ptr;
        ++item_count;
    }
    //CHECK_REPORT_COUNT(item_count);
}

void RtcpSR::setNtpStamp(struct timeval tv) {
    ntpmsw = htonl(tv.tv_sec + 0x83AA7E80); /* 0x83AA7E80 is the number of seconds from 1900 to 1970 */
    ntplsw = htonl((uint32_t)((double)tv.tv_usec * (double)(((uint64_t)1) << 32) * 1.0e-6));
}

void RtcpSR::setNtpStamp(uint64_t unix_stamp_ms) {
    struct timeval tv;
    tv.tv_sec = unix_stamp_ms / 1000;
    tv.tv_usec = (unix_stamp_ms % 1000) * 1000;
    setNtpStamp(tv);
}

std::vector<ReportItem*> RtcpRR::getItemList() {
    std::vector<ReportItem*> ret;
    ReportItem* ptr = &items;
    for (int i = 0; i < (int)report_count; ++i) {
        ret.emplace_back(ptr);
        ++ptr;
    }
    return ret;
}


void RtcpRR::net2Host(size_t size) {
    static const size_t kMinSize = sizeof(RtcpRR) - sizeof(items);
    //CHECK_MIN_SIZE(size, kMinSize);
    ssrc = ntohl(ssrc);

    ReportItem* ptr = &items;
    int item_count = 0;
    for (int i = 0; i < (int)report_count && (char*)(ptr)+sizeof(ReportItem) <= (char*)(this) + size; ++i) {
        ptr->net2Host();
        ++ptr;
        ++item_count;
    }
    //CHECK_REPORT_COUNT(item_count);
}