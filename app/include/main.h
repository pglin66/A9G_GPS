#ifndef __MAIN_H_
#define __MAIN_H_

int Http(const char* domain, int port, const char* path, const char* type, const char* body, char* retBuffer, int bufferLen)
{

	int bodyLen= strlen(body);
	if (!strcmp(type, "POST")) {
		Trace(2, "type:POST");
	}
	
	uint8_t ip[16];
	bool flag = false;
	uint16_t recvLen = 0;

	//connect server
	memset(ip, 0, sizeof(ip));
	if (DNS_GetHostByName2(domain, ip) != 0)
	{
		Trace(2, "get ip error");
		return -1;
	}
	 Trace(2,"get ip success:%s -> %s",domain,ip);
	char* servInetAddr = ip;
	
	
	//int* bodyLen;//post 发送数据需要发送数据的长度
	if (!strcmp(type, "POST")) {

		snprintf(retBuffer, bufferLen, "POST %s HTTP/1.0\r\nContent-Type: application/x-www-form-urlencoded\r\nConnection: Keep-Alive\r\nHost: %s\r\nX-Requested-With:XMLHttpRequest\r\nContent-Length: %d\r\n\r\n", path, domain, bodyLen);
	}
	else {
		snprintf(retBuffer, bufferLen, "GET %s HTTP/1.0\r\nX-Requested-With:XMLHttpRequest\r\nHost: %s\r\n\r\n", path, domain);
	}
	char* pData = retBuffer;
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd < 0) {
		Trace(2, "socket fail");
	
		return -1;
	}
	 Trace(2,"fd:%d",fd);

	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	inet_pton(AF_INET, servInetAddr, &sockaddr.sin_addr);

	int ret = connect(fd, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr_in));
	if (ret < 0) {
		Trace(2, "socket connect fail");
		
		return -1;
	}
	 Trace(2,"socket connect success");
	Trace(2, "send request:%s", pData);
	ret = send(fd, pData, strlen(pData), 0);
	if (ret < 0) {
		Trace(2, "socket send fail");
	
		return -1;
	}
	if (!strcmp(type, "POST")) {
		ret = send(fd, body, bodyLen, 0);
		if (ret < 0) {
			Trace(2, "socket send fail");

			return -1;
		}
	}
	 Trace(2,"socket send success");

	struct fd_set fds;
	struct timeval timeout = { 12,0 };
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	while (!flag)
	{
		ret = select(fd + 1, &fds, NULL, NULL, &timeout);
		 Trace(2,"select return:%d",ret);
		switch (ret)
		{
		case -1:
			Trace(2, "select error");
			flag = true;
			break;
		case 0:
			Trace(2, "select timeout");
			flag = true;
			break;
		default:
			if (FD_ISSET(fd, &fds))
			{
				memset(retBuffer, 0, bufferLen);
				ret = recv(fd, retBuffer, bufferLen, 0);
				recvLen += ret;
				if (ret < 0)
				{
					Trace(2, "recv error");
					flag = true;
					break;
				}
				else if (ret == 0)
				{
					Trace(2, "ret == 0");
					break;
				}
				else if (ret < 1352)
				{
					Trace("recv len:%d,data:%s", recvLen, retBuffer);
					close(fd);
					
					return recvLen;
				}
			}
			break;
		}
	}
	close(fd);
	
	return -1;
}


char * http_body(const char* body) {
	char* index0 = strstr(body, "\r\n\r\n");
	char temp = index0[4];
	index0[4] = '\0';	
	index0[4] = temp;	
	return index0 + 4;
}


#endif

