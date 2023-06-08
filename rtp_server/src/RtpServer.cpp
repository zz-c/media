//
// Created by bxc on 2023/3/2.
//

#include <stdint.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include "Rtp.h"
#include "Rtcp.h"
#include "RtcpContext.h"
#include "Log.h"
#include "Utils.h"
#include "RtpServer.h"

#pragma comment(lib, "ws2_32.lib")

#define CACHE_MAX_SIZE 20000

RtpServer::RtpServer() {
	mRtcpContextForRecv = new RtcpContextForRecv;
	mRecvCache = (uint8_t*)malloc(CACHE_MAX_SIZE);
	//char buf[4];
	//int size = 1440;

	//buf[0] = (uint8_t)(((size) & 0xFF00) >> 8);
	//buf[1] = (uint8_t)((size) & 0xFF);
	//buf[2] = (size >> 8) & 0xFF;
	//buf[3] = size & 0xFF;

}
RtpServer::~RtpServer() {

	delete mRtcpContextForRecv;
	mRtcpContextForRecv = nullptr;

	free(mRecvCache);
	mRecvCache = nullptr;

}

void RtpServer::parseRecvData(char* recvBuf, int recvBufSize) {

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
					struct RtpHeader rtpHeader;
					parseRtpHeader((uint8_t*)pktBuf, &rtpHeader);

					printf("seq=%d,timestamp=%d,ssrc=%d \n",rtpHeader.seq,rtpHeader.timestamp,rtpHeader.ssrc);

					mRtcpContextForRecv->onRtp(rtpHeader.seq, rtpHeader.timestamp, 0, 90000, pktSize);

				}
				else if (0x01 == channel) {
					// RTCP
					printf("RTCP magic=%d,channel=%d,mRecvCacheSize=%d,pktSize=%d\n", magic, channel, mRecvCacheSize, pktSize);

					//for (int i = 0; i < pktSize; i++) {
					//	printf("%d-%d\n", i, pktBuf[i]);
					//}
					std::vector<RtcpHeader*> rtcps = RtcpHeader::loadFromBytes(pktBuf, pktSize);
					for (auto& rtcp : rtcps) {
						if ((RtcpType)rtcp->pt == RtcpType::RTCP_SR) {
							RtcpSR* rtcp_sr = (RtcpSR*)rtcp;
							printf("接收到 rtcp sr----------start\n");
							std::cout << "ssrc:" << rtcp_sr->ssrc << "\n";
							std::cout << "ntpmsw:" << rtcp_sr->ntpmsw << "\n";
							std::cout << "ntplsw:" << rtcp_sr->ntplsw << "\n";
							std::cout << "rtpts:" << rtcp_sr->rtpts << "\n";
							std::cout << "packet_count:" << rtcp_sr->packet_count << "\n";
							std::cout << "octet_count:" << rtcp_sr->octet_count << "\n";
							printf("接收到 rtcp sr----------end\n");
						}
						else {
							LOGE("接收到未定义的RTCP类型");
						
						}
		
						mRtcpContextForRecv->onRtcp(rtcp);

					}
					//printf("x\n");

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

int RtpServer::start(const char* ip, uint16_t port) {

	LOGI("rtpServer rtp://%s:%d", ip, port);

	SOCKET server_fd = -1;
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		LOGI("WSAStartup error");
		return -1;
	}
	SOCKADDR_IN server_addr;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	//server_addr.sin_addr.s_addr = inet_addr("192.168.2.61");
	server_addr.sin_port = htons(port);

	server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (bind(server_fd, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		LOGI("socket bind error");
		return -1;
	}

	if (listen(server_fd, SOMAXCONN) < 0) {
		LOGI("socket listen error");
		return -1;
	}

	while (true)
	{
		LOGI("阻塞监听新连接...");
		// 阻塞接收请求 start
		int len = sizeof(SOCKADDR);
		SOCKADDR_IN accept_addr;
		int clientFd = accept(server_fd, (SOCKADDR*)&accept_addr, &len);
		//const char* clientIp = inet_ntoa(accept_addr.sin_addr);

		if (clientFd == SOCKET_ERROR) {
			LOGI("accept connection error");
			break;
		}
		// 阻塞接收请求 end
		LOGI("发现新连接：clientFd=%d", clientFd);


		{


			char recvBuf[10000];
			int  recvBufSize;

			uint64_t speedTotalSize = 0;
			time_t  t1 = time(NULL);
			time_t  t2 = 0;

			uint64_t last_send_rr_start = getCurTime();


			while (true) {
				recvBufSize = recv(clientFd, recvBuf, sizeof(recvBuf), 0);
				if (recvBufSize <= 0) {
					LOGE("::recv error: clientFd=%d,recvBufSize=%d", clientFd, recvBufSize);
					break;
				}

				time_t cur = getCurTime();

				if ((cur - last_send_rr_start) > 1000) { // 1000毫秒向客户端发送一次RTCP RR
					last_send_rr_start = cur;

					//printf("向推流端发送RTCP RR\n");


					RtcpRR* rtcp = mRtcpContextForRecv->createRtcpRR(mRtcpSSRC, mRtpSSRC);

					printf("向推流端发送 rtcp rr----------start\n");
					std::cout << "rtcp->getSize():" << rtcp->getSize() << "\n";
					std::cout << "ssrc:" << ntohl(rtcp->ssrc) << "\n";
					printf("向推流端发送 rtcp rr----------end\n");


					uint32_t rtcpSize = rtcp->getSize();
					char* rtcpBuf = (char*)malloc(4 + rtcpSize);
					rtcpBuf[0] = 0x24;//$
					rtcpBuf[1] = 0x01;// 0x00;
					rtcpBuf[2] = (uint8_t)(((rtcpSize) & 0xFF00) >> 8);
					rtcpBuf[3] = (uint8_t)((rtcpSize) & 0xFF);

					memcpy(rtcpBuf + 4, (char*)rtcp, rtcpSize);

					int sendRtcpBufSize = ::send(clientFd, rtcpBuf, 4 + rtcpSize, 0);
					if (sendRtcpBufSize <= 0) {
						LOGE("::send rr error : clientFd=%d,sendRtcpBufSize=%d", clientFd, sendRtcpBufSize);
						break;
					}
					free(rtcpBuf);
					rtcpBuf = nullptr;
					delete rtcp;
					rtcp = nullptr;

				}

				speedTotalSize += recvBufSize;
				if (speedTotalSize > 2097152) /* 2097152=2*1024*1024=2mb*/
				{
					t2 = time(NULL);
					if (t2 - t1 > 0) {
						uint64_t speed = speedTotalSize / 1024 / (t2 - t1);
						printf("clientFd=%d,speedTotalSize=%llu,speed=%llu Kbps\n", clientFd, speedTotalSize, speed);
						speedTotalSize = 0;
						t1 = time(NULL);
					}
				}




				parseRecvData(recvBuf, recvBufSize);
			}



		}
		closesocket(clientFd);
		LOGI("关闭连接 clientFd=%d", clientFd);


	}

	return 0;
}