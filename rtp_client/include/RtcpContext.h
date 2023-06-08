//
// Created by bxc on 2023/3/2.
//

#ifndef RTPSERVER_RTCPCONTEXT_H
#define RTPSERVER_RTCPCONTEXT_H

#include "Rtcp.h"
#include <stddef.h>
#include <stdint.h>
#include <map>


class RtcpContext {
public:
    virtual ~RtcpContext() = default;

    /**
     * 输出或输入rtp时调用
     * @param seq rtp的seq
     * @param stamp rtp的时间戳，单位采样数(非毫秒)
     * @param ntp_stamp_ms ntp时间戳
     * @param rtp rtp时间戳采样率，视频一般为90000，音频一般为采样率
     * @param bytes rtp数据长度
     */
    virtual void onRtp(uint16_t seq, uint32_t stamp, uint64_t ntp_stamp_ms, uint32_t sample_rate, size_t bytes);

    /**
     * 输入sr rtcp包
     * @param rtcp 输入一个rtcp
     */
    virtual void onRtcp(RtcpHeader* rtcp) = 0;

    /**
     * 计算总丢包数
     */
    virtual size_t getLost();

    /**
     * 返回理应收到的rtp数
     */
    virtual size_t getExpectedPackets() const;

    /**
     * 创建SR rtcp包
     * @param rtcp_ssrc rtcp的ssrc
     * @return rtcp包
     */
    //virtual toolkit::Buffer::Ptr createRtcpSR(uint32_t rtcp_ssrc);
    virtual RtcpSR* createRtcpSR(uint32_t rtcp_ssrc);
    /**
     * @brief 创建xr的dlrr包，用于接收者估算rtt
     *
     * @return toolkit::Buffer::Ptr
     */
    //virtual toolkit::Buffer::Ptr createRtcpXRDLRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc);

    /**
     * 创建RR rtcp包
     * @param rtcp_ssrc rtcp的ssrc
     * @param rtp_ssrc rtp的ssrc
     * @return rtcp包
     */
    //virtual toolkit::Buffer::Ptr createRtcpRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc);

    virtual RtcpRR* createRtcpRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc);

    /**
     * 上次结果与本次结果间应收包数
     */
    virtual size_t getExpectedPacketsInterval();

    /**
     * 上次结果与本次结果间丢包个数
     */
    virtual size_t geLostInterval();

protected:
    // 收到或发送的rtp的字节数
    size_t _bytes = 0;
    // 收到或发送的rtp的个数
    size_t _packets = 0;
    // 上次的rtp时间戳,毫秒
    uint32_t _last_rtp_stamp = 0;
    uint64_t _last_ntp_stamp_ms = 0;
};

class RtcpContextForSend : public RtcpContext {
public:
    //toolkit::Buffer::Ptr createRtcpSR(uint32_t rtcp_ssrc) override;
    RtcpSR* createRtcpSR(uint32_t rtcp_ssrc) override;
    void onRtcp(RtcpHeader* rtcp) override;

    //toolkit::Buffer::Ptr createRtcpXRDLRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc) override;

    /**
     * 获取rtt
     * @param ssrc rtp ssrc
     * @return rtt,单位毫秒
     */
    uint32_t getRtt(uint32_t ssrc) const;

private:
    std::map<uint32_t /*ssrc*/, uint32_t /*rtt*/> _rtt;
    std::map<uint32_t /*last_sr_lsr*/, uint64_t /*ntp stamp*/> _sender_report_ntp;

    std::map<uint32_t /*ssrc*/, uint64_t /*xr rrtr sys stamp*/> _xr_rrtr_recv_sys_stamp;
    std::map<uint32_t /*ssrc*/, uint32_t /*last rr */> _xr_xrrtr_recv_last_rr;
};

#endif //RTPSERVER_RTCPCONTEXT_H

