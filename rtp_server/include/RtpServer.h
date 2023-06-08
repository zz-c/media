//
// Created by bxc on 2023/3/2.
//

#ifndef BXC_RTPSERVER_H
#define BXC_RTPSERVER_H

#include <stdint.h>
class RtcpContext;

class RtpServer {
public:
	RtpServer();
	~RtpServer();
public:
	int start(const char* ip, uint16_t port);
	void parseRecvData(char* recvBuf, int recvBufSize);
private:
	RtcpContext* mRtcpContextForRecv;
	uint8_t* mRecvCache = nullptr;
	uint64_t mRecvCacheSize = 0;

	uint32_t mRtcpSSRC = 0x09;
	uint32_t mRtpSSRC = 0x08;
};

#endif //BXC_RTPSERVER_H

