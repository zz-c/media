//
// Created by bxc on 2023/3/2.
//


#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>
#include "Rtp.h"
#include "Rtcp.h"
#include "RtcpContext.h"
#include "Log.h"
#include "Utils.h"
#include "RtpClient.h"
#pragma comment(lib, "ws2_32.lib")

#define CACHE_MAX_SIZE 20000


RtpClient::RtpClient() {
	mRtcpContextForSend = new RtcpContextForSend;
	mRecvCache = (uint8_t*)malloc(CACHE_MAX_SIZE);

}
RtpClient::~RtpClient() {
	delete mRtcpContextForSend;
	mRtcpContextForSend = nullptr;

	free(mRecvCache);
	mRecvCache = nullptr;


}
void RtpClient::parseRecvData(char* recvBuf, int recvBufSize) {

	if ((mRecvCacheSize + recvBufSize) > CACHE_MAX_SIZE) {
		LOGE("缓冲数据加上本次接读取数据，超过缓冲容量上限，忽略本次读取的数据。mRecvCacheSize=%d,recvBufSize=%d",
			mRecvCacheSize, recvBufSize);
		//continue;
	}
	else {
		memcpy(mRecvCache + mRecvCacheSize, recvBuf, recvBufSize);
		mRecvCacheSize += recvBufSize;
	}
	//LOGI("cacheSize=%d，开始进入解析 ... ...", cacheSize);

	while (true) {

		if (mRecvCacheSize > RTP_HEADER_SIZE) {
			bool success = false;
			int16_t pktSize; // rtp或rtcp包的总长度
			uint32_t index = 0;
			uint8_t magic;
			uint8_t channel;

			for (index = 0; index < (mRecvCacheSize - RTP_HEADER_SIZE); ++index) {
				magic = mRecvCache[index];

				if (0x24 == magic) {
					channel = mRecvCache[index + 1];
					//uint32_t pktSize;
					//memcpy(&pktSize, cache + index + 2, 2);


					pktSize = ntohs(*(int16_t*)(mRecvCache + index + 2));


					if ((mRecvCacheSize - 4) >= pktSize) {
						success = true;
					}
					break;

				}

			}
			if (success) {
				mRecvCacheSize -= 4;
				mRecvCacheSize -= pktSize;

				// 获取当前数据包的buf
				char* pktBuf = (char*)malloc(pktSize);
				memcpy(pktBuf, mRecvCache + index + 4, pktSize);

				memmove(mRecvCache, mRecvCache + index + 4 + pktSize, mRecvCacheSize);

				if (0x00 == channel) {
					// RTP
					//printf("RTP magic=%d,channel=%d,mRecvCacheSize=%d,pktSize=%d\n", magic, channel, mRecvCacheSize, pktSize);

				}
				else if (0x01 == channel) {
					// RTCP
					printf("RTCP magic=%d,channel=%d,mRecvCacheSize=%d,pktSize=%d\n", magic, channel, mRecvCacheSize, pktSize);

					//for (int i = 0; i < pktSize; i++) {
					//	printf("%d-%d\n", i,pktBuf[i]);
					//}
					auto rtcp_arr = RtcpHeader::loadFromBytes(pktBuf, pktSize);
					for (auto& rtcp : rtcp_arr) {
						mRtcpContextForSend->onRtcp(rtcp);
					}
					//printf("x\n");


					RtcpSR* rtcp = mRtcpContextForSend->createRtcpSR(mRtpSSRC);

					printf("向接收端发送 rtcp sr----------start\n");
					std::cout << "ssrc:" << ntohl(rtcp->ssrc) << "\n";
					std::cout << "ntpmsw:" << ntohl(rtcp->ntpmsw) << "\n";
					std::cout << "ntplsw:" << ntohl(rtcp->ntplsw) << "\n";
					std::cout << "rtpts:" << ntohl(rtcp->rtpts) << "\n";
					std::cout << "packet_count:" << ntohl(rtcp->packet_count) << "\n";
					std::cout << "octet_count:" << ntohl(rtcp->octet_count) << "\n";
					printf("向接收端发送 rtcp sr----------end\n");

					uint32_t rtcpSize = rtcp->getSize();
					char* rtcpBuf = (char*)malloc(4 + rtcpSize);
					rtcpBuf[0] = 0x24;//$
					rtcpBuf[1] = 0x01;// 0x00;
					rtcpBuf[2] = (uint8_t)(((rtcpSize) & 0xFF00) >> 8);
					rtcpBuf[3] = (uint8_t)((rtcpSize) & 0xFF);

					memcpy(rtcpBuf + 4, (char*)rtcp, rtcpSize);

					int sendRtcpBufSize = ::send(mConnFd, rtcpBuf, 4 + rtcpSize, 0);
					if (sendRtcpBufSize <= 0) {
						LOGE("::send sr error : mConnFd=%d,sendRtcpBufSize=%d", mConnFd, sendRtcpBufSize);
						break;
					}
					free(rtcpBuf);
					rtcpBuf = nullptr;
					delete rtcp;
					rtcp = nullptr;


				}
				free(pktBuf);
				pktBuf = nullptr;

			}
			else {
				//LOGI("跳出解析:cacheSize=%d,pktSize=%d", cacheSize, pktSize);
				break;
			}
		}
		else {
			//LOGI("跳出解析:缓冲数据未发现完整数据包");
			break;
		}
	}



}

int RtpClient::start(const char* serverIp, uint16_t serverPort) {

	LOGI("rtpClient start,serverIp=%s,serverPort=%d", serverIp, serverPort);


	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		LOGE("WSAStartup error");
		return -1;
	}

	mConnFd = socket(AF_INET, SOCK_STREAM, 0);
	if (mConnFd == -1)
	{
		LOGE("create socket error");
		WSACleanup();
		return -1;
	}
	int on = 1;
	setsockopt(mConnFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));


	//设置 server_addr
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(serverPort);
	//server_addr.sin_addr.s_addr = inet_addr(serverIp);
	inet_pton(AF_INET, serverIp, &server_addr.sin_addr);

	if (connect(mConnFd, (struct sockaddr*)&server_addr, sizeof(sockaddr_in)) == -1)
	{
		LOGE("socket connect error");
		return -1;
	}
	LOGI("mConnFd=%d connect success", mConnFd);

	// 发送数据子线程
	std::thread t1([&]() {
		struct RtpPacket* rtpPacket = (struct RtpPacket*)malloc(500000);
		rtpHeaderInit(rtpPacket, 0, 0, 0, RTP_VESION, RTP_PAYLOAD_TYPE_H264, 0,
			0, 0, mRtpSSRC);


		// 模拟封装在RTP包中的视频流数据
		char frame[4000];
		int  frameSize = sizeof(frame);
		memset(frame, 0, frameSize);

		while (true)
		{

			rtpPacket->rtpHeader.seq++;
			
			rtpPacket->rtpHeader.timestamp += 90000 / 25;

			memcpy(rtpPacket->payload, frame, frameSize);

			uint32_t rtpSize = RTP_HEADER_SIZE + frameSize;

			// RTP发送前
			rtpPacket->rtpHeader.seq = htons(rtpPacket->rtpHeader.seq);
			rtpPacket->rtpHeader.timestamp = htonl(rtpPacket->rtpHeader.timestamp);
			rtpPacket->rtpHeader.ssrc = htonl(rtpPacket->rtpHeader.ssrc);

			char* tempBuf = (char*)malloc(4 + rtpSize);
			tempBuf[0] = 0x24;//$
			tempBuf[1] = 0x00;// 0x00;
			tempBuf[2] = (uint8_t)(((rtpSize) & 0xFF00) >> 8);
			tempBuf[3] = (uint8_t)((rtpSize) & 0xFF);
			memcpy(tempBuf + 4, (char*)rtpPacket, rtpSize);

			int sendRtpSize = ::send(mConnFd, tempBuf, 4 + rtpSize, 0);
			free(tempBuf);
			tempBuf = nullptr;


			if (sendRtpSize <= 0) {

				LOGE("::send rtp error: mConnFd=%d,sendRtpSize=%d", mConnFd, sendRtpSize);
				break;
			}

			// RTP发送后
			rtpPacket->rtpHeader.seq = ntohs(rtpPacket->rtpHeader.seq);
			rtpPacket->rtpHeader.timestamp = ntohl(rtpPacket->rtpHeader.timestamp);
			rtpPacket->rtpHeader.ssrc = ntohl(rtpPacket->rtpHeader.ssrc);


			// 设置RtcpSR
			uint64_t ntp_stamp_ms = getCurrentMillisecond();

			mRtcpContextForSend->onRtp(
				rtpPacket->rtpHeader.seq,
				rtpPacket->rtpHeader.timestamp,
				ntp_stamp_ms,
				90000 /*not used*/,
				rtpSize);

			Sleep(40);
		}

		});

	// 接收数据子线程
	std::thread t2([&]() {

		char recvBuf[1000];
		int  recvBufSize;

		while (true) {
			recvBufSize = recv(mConnFd, recvBuf, sizeof(recvBuf), 0);
			if (recvBufSize <= 0) {
				LOGE("::recv error: mConnFd=%d,recvBufSize=%d", mConnFd, recvBufSize);
				break;
			}

			//LOGI("recvBufSize=%d", recvBufSize);

			parseRecvData(recvBuf, recvBufSize);

		}

		});
	t1.join();
	t2.join();

	closesocket(mConnFd);
	mConnFd;

	return 0;
}