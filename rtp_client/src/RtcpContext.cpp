//
// Created by bxc on 2023/3/2.
//

#include "RtcpContext.h"
#include "Utils.h"
#include <stdio.h>
#include <string.h>

void RtcpContext::onRtp(
    uint16_t /*seq*/, uint32_t stamp, uint64_t ntp_stamp_ms, uint32_t /*sample_rate*/, size_t bytes) {
    ++_packets;
    _bytes += bytes;
    _last_rtp_stamp = stamp;
    _last_ntp_stamp_ms = ntp_stamp_ms;
}

size_t RtcpContext::getExpectedPackets() const {
    throw std::runtime_error("没有实现, rtp发送者无法统计应收包数");
}

size_t RtcpContext::getExpectedPacketsInterval() {
    throw std::runtime_error("没有实现, rtp发送者无法统计应收包数");
}

size_t RtcpContext::getLost() {
    throw std::runtime_error("没有实现, rtp发送者无法统计丢包率");
}

size_t RtcpContext::geLostInterval() {
    throw std::runtime_error("没有实现, rtp发送者无法统计丢包率");
}

//Buffer::Ptr RtcpContext::createRtcpSR(uint32_t rtcp_ssrc) {
//    throw std::runtime_error("没有实现, rtp接收者尝试发送sr包");
//}
RtcpSR* RtcpContext::createRtcpSR(uint32_t rtcp_ssrc){
   throw std::runtime_error("没有实现, rtp接收者尝试发送sr包");
}

//Buffer::Ptr RtcpContext::createRtcpRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc) {
//    throw std::runtime_error("没有实现, rtp发送者尝试发送rr包");
//}
RtcpRR* RtcpContext::createRtcpRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc) {
    throw std::runtime_error("没有实现, rtp发送者尝试发送rr包");
}
 
//
//Buffer::Ptr RtcpContext::createRtcpXRDLRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc) {
//    throw std::runtime_error("没有实现, rtp发送者尝试发送xr dlrr包");
//}

////////////////////////////////////////////////////////////////////////////////////

void RtcpContextForSend::onRtcp(RtcpHeader* rtcp) {
    switch ((RtcpType)rtcp->pt) {
    case RtcpType::RTCP_RR: {
        auto rtcp_rr = (RtcpRR*)rtcp;
     
        for (auto item : rtcp_rr->getItemList()) {
            if (!item->last_sr_stamp) {
                continue;
            }
            auto it = _sender_report_ntp.find(item->last_sr_stamp);
            if (it == _sender_report_ntp.end()) {
                continue;
            }
            // 发送sr到收到rr之间的时间戳增量
            auto ms_inc = getCurrentMillisecond() - it->second;
            // rtp接收端收到sr包后，回复rr包的延时，已转换为毫秒
            auto delay_ms = (uint64_t)item->delay_since_last_sr * 1000 / 65536;
            auto rtt = (int)(ms_inc - delay_ms);
            if (rtt >= 0) {
                // rtt不可能小于0
                _rtt[item->ssrc] = rtt;
                // InfoL << "ssrc:" << item->ssrc << ",rtt:" << rtt;
            }
        }
    
        break;
    }
    default:
        break;
    }
}

uint32_t RtcpContextForSend::getRtt(uint32_t ssrc) const {
    auto it = _rtt.find(ssrc);
    if (it == _rtt.end()) {
        return 0;
    }
    return it->second;
}
RtcpSR* RtcpContextForSend::createRtcpSR(uint32_t rtcp_ssrc) {

    size_t item_count = 0;//SR包扩展0个ReportItem
    auto real_size = sizeof(RtcpSR) - sizeof(ReportItem) + item_count * sizeof(ReportItem);
    auto bytes = alignSize(real_size);

    auto rtcp = new RtcpSR;
    setupHeader(rtcp, RtcpType::RTCP_SR, item_count, bytes);
    setupPadding(rtcp, bytes - real_size);

    rtcp->ntpmsw = 0;
    rtcp->ntplsw = 0;
    rtcp->setNtpStamp(_last_ntp_stamp_ms);

    rtcp->rtpts = htonl(_last_rtp_stamp);
    rtcp->ssrc = htonl(rtcp_ssrc);
    rtcp->packet_count = htonl((uint32_t)_packets);
    rtcp->octet_count = htonl((uint32_t)_bytes);

    // 记录上次发送的sender report信息，用于后续统计rtt
    auto last_sr_lsr = ((ntohl(rtcp->ntpmsw) & 0xFFFF) << 16) | ((ntohl(rtcp->ntplsw) >> 16) & 0xFFFF);
    _sender_report_ntp[last_sr_lsr] = getCurrentMillisecond();
    if (_sender_report_ntp.size() >= 5) {
        // 删除最早的sr rtcp
        _sender_report_ntp.erase(_sender_report_ntp.begin());
    }

    return rtcp;
}

