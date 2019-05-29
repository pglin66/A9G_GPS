#ifndef __MAIN_H_
#define __MAIN_H_

int Http_Get(const char* domain, int port, const char* path, char* retBuffer)
{
	int bufferLen = sizeof(retBuffer);
	bool flag = false;
	uint16_t recvLen = 0;
	uint8_t ip[16];

	//connect server
	memset(ip, 0, sizeof(ip));
	if (DNS_GetHostByName2(domain, ip) != 0)
	{
		Trace(1, "get ip error");
		return -1;
	}
	Trace(1, "get ip success:%s -> %s", domain, ip);
	char* servInetAddr = ip;
	snprintf(retBuffer, bufferLen, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", path, domain);
	char* pData = retBuffer;
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd < 0) {
		Trace(1, "socket fail");
		return -1;
	}
	Trace(1, "fd:%d", fd);

	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	inet_pton(AF_INET, servInetAddr, &sockaddr.sin_addr);

	int ret = connect(fd, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr_in));
	if (ret < 0) {
		Trace(1, "socket connect fail");
		return -1;
	}
	Trace(1, "socket connect success");
	Trace(1, "send request:%s", pData);
	ret = send(fd, pData, strlen(pData), 0);
	if (ret < 0) {
		Trace(1, "socket send fail");
		return -1;
	}
	Trace(1, "socket send success");

	struct fd_set fds;
	struct timeval timeout = { 12,0 };
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	while (!flag)
	{
		ret = select(fd + 1, &fds, NULL, NULL, &timeout);
		switch (ret)
		{
		case -1:
			Trace(1, "select error");
			flag = true;
			break;
		case 0:
			Trace(1, "select timeout");
			flag = true;
			break;
		default:
			if (FD_ISSET(fd, &fds))
			{
				Trace(1, "select return:%d", ret);
				memset(retBuffer + recvLen, 0, bufferLen - recvLen);
				ret = recv(fd, retBuffer + recvLen, bufferLen - recvLen, 0);
				Trace(1, "ret:%d", ret);
				recvLen += ret;
				if (ret < 0)
				{
					Trace(1, "recv error");
					flag = true;
					break;
				}
				else if (ret == 0)
				{
					Trace(1, "ret == 0");
					flag = true;
					break;
				}
				else if (ret < 1352)
				{
					Trace(1, "recv len:%d,data:%s", recvLen, retBuffer);
					bufferLen = recvLen;
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


int Http(const char* domain, int port, const char* path, const char* type, const char* body, char* retBuffer)
{
	int bufferLen= sizeof(retBuffer);//获取接受固定长度
	bool flag = false;
	uint16_t recvLen = 0;
	uint8_t ip[16];
	//connect server
	memset(ip, 0, sizeof(ip));
	DNS_GetHostByName2(domain, ip);
	if (DNS_GetHostByName2(domain, ip) != 0)
	{
		Trace(1, "get ip error");
		return -1;
	}
	Trace(1, "get ip success:%s -> %s", domain, ip);
	char* servInetAddr = ip;
	//int* bodyLen;//post 发送数据需要发送数据的长度
	if (!strcmp(type, "POST")) {
		//bodyLen = sizeof(body);
		
	     snprintf(retBuffer, bufferLen, "POST %s HTTP/1.1\r\nContent-Type: application/json\r\nConnection: Keep-Alive\r\nHost: %s\r\nContent-Length: %d\r\n\r\n", path, domain, sizeof(body));
	}
	else {
		snprintf(retBuffer, bufferLen, "GET %s%s HTTP/1.0\r\nHost: %s\r\n\r\n", path,, domain);
	}

	
	char* pData = retBuffer;
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd < 0) {
		Trace(1, "socket fail");
		return -1;
	}
	Trace(1, "fd:%d", fd);

	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	inet_pton(AF_INET, servInetAddr, &sockaddr.sin_addr);

	int ret = connect(fd, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr_in));
	if (ret < 0) {
		Trace(1, "socket connect fail");
		return -1;
	}
	Trace(1, "socket connect success");
	Trace(1, "send request:%s", pData);
	ret = send(fd, pData, strlen(pData), 0);

	if (ret < 0) {
		Trace(1, "socket send fail");
		return -1;
	}
	if (!strcmp(type, "POST")) {//post 需要第二次发送
		ret = send(fd, body, sizeof(body), 0);
		if (ret < 0) {
			Trace(2, "socket send fail");
			//OS_Free(temp);
			return -1;
		}
	}
	Trace(1, "socket send success");
	struct fd_set fds;
	struct timeval timeout = { 12,0 };
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	while (!flag)
	{
		ret = select(fd + 1, &fds, NULL, NULL, &timeout);


		switch (ret)
		{
		case -1:
			Trace(1, "select error");
			flag = true;
			break;
		case 0:
			Trace(1, "select timeout");
			flag = true;
			break;
		default:
			if (FD_ISSET(fd, &fds))
			{
				Trace(1, "select return:%d", ret);
				memset(retBuffer + recvLen, 0, bufferLen - recvLen);
				ret = recv(fd, retBuffer + recvLen, bufferLen - recvLen, 0);
				Trace(1, "ret:%d", ret);
				recvLen += ret;
				if (ret < 0)
				{
					Trace(1, "recv error");
					flag = true;
					break;
				}
				else if (ret == 0)
				{
					Trace(1, "ret == 0");
					flag = true;
					break;
				}
				else if (ret < 1352)
				{
					Trace(1, "recv len:%d,data:%s", recvLen, retBuffer);
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


int Http_Post(const char* domain, int port, const char* path, uint8_t* body, uint16_t bodyLen, char* retBuffer, int bufferLen)
{
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
	// Trace(2,"get ip success:%s -> %s",domain,ip);
	char* servInetAddr = ip;
	char* temp = OS_Malloc(2048);
	if (!temp)
	{
		Trace(2, "malloc fail");
		return -1;
	}
	snprintf(temp, 2048, "POST %s HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\nConnection: Keep-Alive\r\nHost: %s\r\nContent-Length: %d\r\n\r\n", path, domain, bodyLen);
	char* pData = temp;
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd < 0) {
		Trace(2, "socket fail");
		OS_Free(temp);
		return -1;
	}
	// Trace(2,"fd:%d",fd);

	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	inet_pton(AF_INET, servInetAddr, &sockaddr.sin_addr);

	int ret = connect(fd, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr_in));
	if (ret < 0) {
		Trace(2, "socket connect fail");
		OS_Free(temp);
		return -1;
	}
	// Trace(2,"socket connect success");
	Trace(2, "send request:%s", pData);
	ret = send(fd, pData, strlen(pData), 0);
	if (ret < 0) {
		Trace(2, "socket send fail");
		OS_Free(temp);
		return -1;
	}
	ret = send(fd, body, bodyLen, 0);
	if (ret < 0) {
		Trace(2, "socket send fail");
		OS_Free(temp);
		return -1;
	}
	// Trace(2,"socket send success");

	struct fd_set fds;
	struct timeval timeout = { 12,0 };
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	while (!flag)
	{
		ret = select(fd + 1, &fds, NULL, NULL, &timeout);
		// Trace(2,"select return:%d",ret);
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
					OS_Free(temp);
					return recvLen;
				}
			}
			break;
		}
	}
	close(fd);
	OS_Free(temp);
	return -1;
}


#endif

