//
// Created by bxc on 2023/3/2.
//

#ifndef RTPSERVER_RTCP_H
#define RTPSERVER_RTCP_H

#include <stdint.h>
#include <string>
#include <vector>
#include <sstream>
#include <WinSock2.h>


#ifndef PACKED
#if !defined(_WIN32)
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif //! defined(_WIN32)
#endif

// http://www.networksorcery.com/enp/protocol/rtcp.htm
#define RTCP_PT_MAP(XX)                                                                                                \
    XX(RTCP_FIR, 192)                                                                                                  \
    XX(RTCP_NACK, 193)                                                                                                 \
    XX(RTCP_SMPTETC, 194)                                                                                              \
    XX(RTCP_IJ, 195)                                                                                                   \
    XX(RTCP_SR, 200)                                                                                                   \
    XX(RTCP_RR, 201)                                                                                                   \
    XX(RTCP_SDES, 202)                                                                                                 \
    XX(RTCP_BYE, 203)                                                                                                  \
    XX(RTCP_APP, 204)                                                                                                  \
    XX(RTCP_RTPFB, 205)                                                                                                \
    XX(RTCP_PSFB, 206)                                                                                                 \
    XX(RTCP_XR, 207)                                                                                                   \
    XX(RTCP_AVB, 208)                                                                                                  \
    XX(RTCP_RSI, 209)                                                                                                  \
    XX(RTCP_TOKEN, 210)


// rtcp类型枚举
enum class RtcpType : uint8_t {
#define XX(key, value) key = value,
    RTCP_PT_MAP(XX)
#undef XX
};


class RtcpHeader {
public:

    // reception report count
    uint32_t report_count : 5;
    // padding，末尾是否有追加填充
    uint32_t padding : 1;
    // 版本号，固定为2
    uint32_t version : 2;

    // rtcp类型,RtcpType
    uint32_t pt : 8;

private:
    // 长度
    uint32_t length : 16; // 始终保存为主机序
public:
    /**
     * 解析rtcp并转换网络字节序为主机字节序，返回RtcpHeader派生类列表
     * @param data 数据指针
     * @param size 数据总长度
     * @return rtcp对象列表，无需free
     */
    static std::vector<RtcpHeader*> loadFromBytes(char* data, size_t size);

    /**
     * 根据length字段获取rtcp总长度
     */
    size_t getSize() const;

    /**
     * 后面追加padding数据长度
     */
    size_t getPaddingSize() const;

    /**
     * 设置rtcp length字段
     * @param size rtcp总长度，单位字节
     */
    void setSize(size_t size);


private:
    /**
     * 调用派生类的net2Host函数
     * @param size rtcp字符长度
     */
    void net2Host(size_t size);
};

// ReportBlock
class ReportItem {
public:
    friend class RtcpSR;
    friend class RtcpRR;

    uint32_t ssrc;
    // Fraction lost
    uint32_t fraction : 8;// 丢包率，从收到上一个SR或RR包以来的RTP数据包的丢失率

    // Cumulative number of packets lost
    uint32_t cumulative : 24; // 累计丢失的数据包数

    // Sequence number cycles count
    uint16_t seq_cycles; // 序列号循环计数

    // Highest sequence number received
    uint16_t seq_max; // 序列最大值

    // Interarrival jitter
    uint32_t jitter;//接收抖动，RTP数据包接受时间的统计方差估计

    // Last SR timestamp, NTP timestamp,(ntpmsw & 0xFFFF) << 16  | (ntplsw >> 16) & 0xFFFF)
    uint32_t last_sr_stamp;//上次SR时间戳，取最近收到的SR包中的NTP时间戳的中间32比特。如果目前还没收到SR包，则该域清零

    // Delay since last SR timestamp,expressed in units of 1/65536 seconds
    uint32_t delay_since_last_sr;//上次SR以来的延时，上次收到SR包到发送本报告的延时
private:
    /**
     * 网络字节序转换为主机字节序
     */
    void net2Host();
} PACKED;

// sender report
class RtcpSR : public RtcpHeader {
public:
    friend class RtcpHeader;
    uint32_t ssrc;
    // ntp timestamp MSW(in second)  秒
    uint32_t ntpmsw = 0;
    // ntp timestamp LSW(in picosecond)  微微秒
    uint32_t ntplsw = 0;
    // rtp timestamp 与RTP的timestamp对应
    uint32_t rtpts = 0;
    // sender packet count 发送RTP数据包的数量
    uint32_t packet_count = 0;
    // sender octet count  发送RTP数据包的字节数
    uint32_t octet_count = 0;
    // 可能有很多个 可扩展，也可以不需要
    ReportItem items;
public:
    /**
* 设置ntpmsw与ntplsw字段为网络字节序
* @param tv 时间
*/
    void setNtpStamp(struct timeval tv);
    void setNtpStamp(uint64_t unix_stamp_ms);

private:

    /**
     * 网络字节序转换为主机字节序
     * @param size 字节长度，防止内存越界
     */
    void net2Host(size_t size);




} PACKED;

// Receiver Report
class RtcpRR : public RtcpHeader {
public:
    friend class RtcpHeader;

    uint32_t ssrc;
    // 可能有很多个
    ReportItem items;

public:

    /**
     * 获取ReportItem对象指针列表
     * 使用net2Host转换成主机字节序后才可使用此函数
     */
    std::vector<ReportItem*> getItemList();

private:
    /**
     * 网络字节序转换为主机字节序
     * @param size 字节长度，防止内存越界
     */
    void net2Host(size_t size);


} PACKED;

//对齐
static size_t alignSize(size_t bytes) {
    return (size_t)((bytes + 3) >> 2) << 2;
}
static void setupHeader(RtcpHeader* rtcp, RtcpType type, size_t report_count, size_t total_bytes) {
    rtcp->version = 2;
    rtcp->padding = 0;
    if (report_count > 0x1F) {
        printf("rtcp report_count最大赋值为31,当前为:%lld", report_count);
        return;
    }
    // items总个数
    rtcp->report_count = report_count;
    rtcp->pt = (uint8_t)type;
    rtcp->setSize(total_bytes);
}
static void setupPadding(RtcpHeader* rtcp, size_t padding_size) {
    if (padding_size) {
        rtcp->padding = 1;
        ((uint8_t*)rtcp)[rtcp->getSize() - 1] = padding_size & 0xFF;
    }
    else {
        rtcp->padding = 0;
    }
}


#endif //RTPSERVER_RTCP_H

