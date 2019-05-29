
#include <string.h>
#include <stdio.h>
#include "assert.h"
#include "buffer.h"
#include "api_debug.h"
#include "api_fs.h"

#include "api_socket.h"
#include "api_os.h"
#include "include/http.h"

//http get with no header
int Http_Get(const char* domain, int port,const char* path, char* retBuffer, int* bufferLen)
{
    bool flag = false;
    uint16_t recvLen = 0;
    uint8_t ip[16];
    int retBufferLen = *bufferLen;
    //connect server
    memset(ip,0,sizeof(ip));
    if(DNS_GetHostByName2(domain,ip) != 0)
    {
        Trace(1,"get ip error");
        return -1;
    }
    Trace(1,"get ip success:%s -> %s",domain,ip);
    char* servInetAddr = ip;
    snprintf(retBuffer,retBufferLen,"GET %s HTTP/1.1\r\nHost: %s\r\n\r\n",path,domain);
    char* pData = retBuffer;
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(fd < 0){
        Trace(1,"socket fail");
        return -1;
    }
    Trace(1,"fd:%d",fd);

    struct sockaddr_in sockaddr;
    memset(&sockaddr,0,sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    inet_pton(AF_INET,servInetAddr,&sockaddr.sin_addr);

    int ret = connect(fd, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr_in));
    if(ret < 0){
        Trace(1,"socket connect fail");
        return -1;
    }
    Trace(1,"socket connect success");
    Trace(1,"send request:%s",pData);
    ret = send(fd, pData, strlen(pData), 0);
    if(ret < 0){
        Trace(1,"socket send fail");
        return -1;
    }
    Trace(1,"socket send success");

    struct fd_set fds;
    struct timeval timeout={12,0};
    FD_ZERO(&fds);
    FD_SET(fd,&fds);
    while(!flag)
    {
        ret = select(fd+1,&fds,NULL,NULL,&timeout);
        switch(ret)
        {
            case -1:
                Trace(1,"select error");
                flag = true;
                break;
            case 0:
                Trace(1,"select timeout");
                flag = true;
                break;
            default:
                if(FD_ISSET(fd,&fds))
                {
                    Trace(1,"select return:%d",ret);
                    memset(retBuffer+recvLen,0,retBufferLen-recvLen);
                    ret = recv(fd,retBuffer+recvLen,retBufferLen-recvLen,0);
                    Trace(1,"ret:%d",ret);
                    recvLen += ret;
                    if(ret < 0)
                    {
                        Trace(1,"recv error");
                        flag = true;
                        break;
                    }
                    else if(ret == 0)
                    {
                        Trace(1,"ret == 0");
                        flag = true;
                        break;
                    }
                    else if(ret < 1352)
                    {
                        Trace(1,"recv len:%d,data:%s",recvLen,retBuffer);
                        *bufferLen = recvLen;
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


//http post with no header
int Http_Post(const char* domain, int port,const char* path,uint8_t* body, uint16_t bodyLen, char* retBuffer, int bufferLen)
{
    uint8_t ip[16];
    bool flag = false;
    uint16_t recvLen = 0;

    //connect server
    memset(ip,0,sizeof(ip));
    Trace(2,"domain:%s",domain);
    if(DNS_GetHostByName2(domain,ip) != 0)
    {
        Trace(2,"get ip error");
        return -1;
    }
     Trace(2,"get ip success:%s -> %s",domain,ip);
    char* servInetAddr = ip;
    char* temp = OS_Malloc(2048);
    if(!temp)
    {
        Trace(2,"malloc fail");
        return -1;
    }
    snprintf(temp,2048,"POST %s HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\nConnection: Keep-Alive\r\nHost: %s\r\nContent-Length: %d\r\n\r\n",
                            path,domain,bodyLen);
    char* pData = temp;
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(fd < 0){
        Trace(2,"socket fail");
        OS_Free(temp);
        return -1;
    }
     Trace(2,"fd:%d",fd);

    struct sockaddr_in sockaddr;
    memset(&sockaddr,0,sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    inet_pton(AF_INET,servInetAddr,&sockaddr.sin_addr);

    int ret = connect(fd, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr_in));
    if(ret < 0){
        Trace(2,"socket connect fail");
        OS_Free(temp);
        return -1;
    }
     Trace(2,"socket connect success");
    Trace(2,"send request:%s",pData);
    ret = send(fd, pData, strlen(pData), 0);
    if(ret < 0){
        Trace(2,"socket send fail");
        OS_Free(temp);
        return -1;
    }
    ret = send(fd, body, bodyLen, 0);
    if(ret < 0){
        Trace(2,"socket send fail");
        OS_Free(temp);
        return -1;
    }
     Trace(2,"socket send success");

    struct fd_set fds;
    struct timeval timeout={12,0};
    FD_ZERO(&fds);
    FD_SET(fd,&fds);
    while(!flag)
    {
        ret = select(fd+1,&fds,NULL,NULL,&timeout);
         Trace(2,"select return:%d",ret);
        switch(ret)
        {
            case -1:
                Trace(2,"select error");
                flag = true;
                break;
            case 0:
                Trace(2,"select timeout");
                flag = true;
                break;
            default:
                if(FD_ISSET(fd,&fds))
                {
                    memset(retBuffer,0,bufferLen);
                    ret = recv(fd,retBuffer,bufferLen,0);
                    recvLen += ret;
                    if(ret < 0)
                    {
                        Trace(2,"recv error");
                        flag = true;
                        break;
                    }
                    else if(ret == 0)
                    {
                        Trace(2,"ret == 0");
                        break;
                    }
                    else if(ret < 1352)
                    {
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

int Http_PostBin(const char* domain, int port,const char* path,char* fileName, char* retBuffer, int bufferLen)
{
    uint8_t ip[16];
    bool flag = false;
    uint16_t recvLen = 0;

    //connect server
    memset(ip, 0, sizeof(ip));
    Trace(2, "domain:%s", domain);
    if (DNS_GetHostByName2(domain, ip) != 0)
    {
        Trace(2, "get ip error");
        return -1;
    }
    Trace(2, "get ip success:%s -> %s", domain, ip);
    char *servInetAddr = ip;
    char *temp = OS_Malloc(2048);
    if (!temp)
    {
        Trace(2, "malloc fail");
        return -1;
    }

    int32_t fdFile;
    fdFile = API_FS_Open(fileName, (FS_O_RDONLY | FS_O_CREAT), 0);
    if (fdFile < 0)
    {
        Trace(2, "Open file failed:%d", fdFile);
        return -1;
    }
    uint64_t bodyLen = API_FS_GetFileSize(fdFile);
    Trace(2,"bodyLen:%lld",bodyLen);

    snprintf(temp, 2048, "POST %s HTTP/1.1\r\nContent-Type: application/octet-stream\r\nConnection: Keep-Alive\r\nHost: %s\r\nContent-Length: %d\r\n\r\n",
             path, domain, (uint32_t)bodyLen);
    char *pData = temp;
    Trace(2,"%s",pData);

    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0)
    {
        Trace(2, "socket fail");
        OS_Free(temp);
        return -1;
    }
    Trace(2, "fd:%d", fd);

    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    inet_pton(AF_INET, servInetAddr, &sockaddr.sin_addr);

    int ret = connect(fd, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in));
    if (ret < 0)
    {
        Trace(2, "socket connect fail");
        OS_Free(temp);
        return -1;
    }
    Trace(2, "socket connect success");
    Trace(2, "send request:%s", pData);
    ret = send(fd, pData, strlen(pData), 0);
    if (ret < 0)
    {
        Trace(2, "socket send fail");
        OS_Free(temp);
        return -1;
    }

    Trace(2, "send body...");
    int32_t retFile;
    uint32_t bufSize = 1024;
    uint8_t body[bufSize];
    uint64_t lll = 0;
    retFile = API_FS_Read(fdFile, (uint8_t *)body, sizeof(body));
    while (retFile > 0)
    {
        lll += retFile;
        ret = send(fd, body, retFile, 0);
        if (ret < 0)
        {
            Trace(2, "socket send fail");
            OS_Free(temp);
            return -1;
        }
        retFile = API_FS_Read(fdFile, body, bufSize);
    }
    API_FS_Close(fdFile);

    Trace(2, "socket send success");

    struct fd_set fds;
    struct timeval timeout = {12, 0};
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    while (!flag)
    {
        ret = select(fd + 1, &fds, NULL, NULL, &timeout);
        Trace(2, "select return:%d", ret);
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