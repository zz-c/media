#include "RtpClient.h"

int main() {
	const char* serverIp = "127.0.0.1";
	const int serverPort = 10005;

	RtpClient rtpClient;
	rtpClient.start(serverIp,serverPort);

	return 0;
}

