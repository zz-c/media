#include "RtpServer.h"

int main() {

	const char* ip = "127.0.0.1";
	uint16_t port = 10005;

	RtpServer rtpServer;
	rtpServer.start(ip, port);

	return 0;
}