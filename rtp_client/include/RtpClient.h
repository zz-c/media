//
// Created by bxc on 2023/3/2.
//

#ifndef BXC_RTPCLIENT_H
#define BXC_RTPCLIENT_H

#include <stdint.h>
class RtcpContext;

class RtpClient {
public:
	RtpClient();
	~RtpClient();
public:
	int start(const char* serverIp, uint16_t serverPort);

	void parseRecvData(char* recvBuf, int recvBufSize);
private:
	int mConnFd = -1;
	RtcpContext* mRtcpContextForSend = nullptr;
	RtcpContext* mRtcpContextForRecv = nullptr;
	uint8_t* mRecvCache = nullptr;
	uint64_t mRecvCacheSize = 0;

	uint32_t mRtpSSRC = 0x08;
};

#endif //BXC_RTPCLIENT_H

