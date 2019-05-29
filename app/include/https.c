#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"

#include "api_os.h"
#include "api_debug.h"
#include "api_event.h"
#include "api_network.h"
#include "api_ssl.h"
#include "api_fs.h"

#include "include/https.h"

#define SSL_SERVER_ADDRESS    "safemq-test.hh-medic.com"
#define SSL_SERVER_PORT       "443"
#define SSL_SERVER_HOST_NAME  "safemq-test.hh-medic.com"

const char* ca_cert = "-----BEGIN CERTIFICATE-----\n\
MIID9zCCAt+gAwIBAgIJAJ/FqkkWhVVtMA0GCSqGSIb3DQEBCwUAMIGRMQswCQYD\n\
VQQGEwJDTjESMBAGA1UECAwJR1VBTkdET05HMREwDwYDVQQHDAhTSE5FWkhFTjEU\n\
MBIGA1UECgwLTkVVQ1JBQ0tfQ0ExCzAJBgNVBAsMAkNBMRQwEgYDVQQDDAtuZXVj\n\
cmFja19jYTEiMCAGCSqGSIb3DQEJARYTY3pkNjY2NjY2QGdtYWlsLmNvbTAeFw0x\n\
ODAxMTgwNzEyMDFaFw0xOTAxMTgwNzEyMDFaMIGRMQswCQYDVQQGEwJDTjESMBAG\n\
A1UECAwJR1VBTkdET05HMREwDwYDVQQHDAhTSE5FWkhFTjEUMBIGA1UECgwLTkVV\n\
Q1JBQ0tfQ0ExCzAJBgNVBAsMAkNBMRQwEgYDVQQDDAtuZXVjcmFja19jYTEiMCAG\n\
CSqGSIb3DQEJARYTY3pkNjY2NjY2QGdtYWlsLmNvbTCCASIwDQYJKoZIhvcNAQEB\n\
BQADggEPADCCAQoCggEBAOFA7NEQvrCpV67exKjyru6Vc2KYFc+B/nQyJmT31nhO\n\
f8De2mUr1HPlZY9u1pJeMCkY2c2ToKXdaXNTUznI3PVTtxc5z2U5u1L2elg7G7kV\n\
zyrAAjz4SKD/hZTL4gUFyO3sfSUBgvDHVwDcB1wqcQPJXmPDRb5aT69LkmHTKMpB\n\
OjYQ3Sw2fQo4EzTjb4lfaAU30MdO1AgfVT0LP82G/7HfUEt2IdZrpjkprHvUo/2m\n\
gDYbq9uOJTCi5HUMSqXDM/wpEnRP4ATSrcL6Xccjryv6wQRroW3da+hXxcWARqZR\n\
i3OfXdCMrSaw8nMNxLPss4UrpQ02P+2yqzWfdeYZ6jkCAwEAAaNQME4wHQYDVR0O\n\
BBYEFJD9fZdjm+lPykElhBviFcYvK3m9MB8GA1UdIwQYMBaAFJD9fZdjm+lPykEl\n\
hBviFcYvK3m9MAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBAA6o/esx\n\
Ls/gftzuWt3b0Wej2GVRk4snx1DAYMnQA3MgAXw1Y3cGZ7y8cwMyDWy7//K2l2kQ\n\
HT6sithRUCBLK3ZkXAAZz5valBdwHSRYWk40oi+WXX0ZzVzsrcf4ISMvk57yl0yV\n\
d91KhrYTFGIksWJR6UMEeVRRQKcHcsjjqsAL0i1rWHHtp3G3EX4LCK91aVKfUWZa\n\
evTKVXCux8nhKo3fDUQTDwNEGd+XDUBUJ/sANhvRJ1zrtT8NbUF5XTIB07r28BsY\n\
DHFqwQjorWeg4Z6t9/yFiffyVmF72MZG7cG24JN+VwEfzD1vz+5OSY0ZeoLUmYv7\n\
NYYZjpkEaGH9eCs=\n\
-----END CERTIFICATE-----";

const char* client_cert = "-----BEGIN CERTIFICATE-----\n\
MIIDpDCCAowCAQEwDQYJKoZIhvcNAQELBQAwgZExCzAJBgNVBAYTAkNOMRIwEAYD\n\
VQQIDAlHVUFOR0RPTkcxETAPBgNVBAcMCFNITkVaSEVOMRQwEgYDVQQKDAtORVVD\n\
UkFDS19DQTELMAkGA1UECwwCQ0ExFDASBgNVBAMMC25ldWNyYWNrX2NhMSIwIAYJ\n\
KoZIhvcNAQkBFhNjemQ2NjY2NjZAZ21haWwuY29tMB4XDTE4MDExODA3MTMyM1oX\n\
DTE5MDExODA3MTMyM1owgZ0xCzAJBgNVBAYTAkNOMRIwEAYDVQQIDAlHVUFOR0RP\n\
TkcxETAPBgNVBAcMCFNIRU5aSEVOMRgwFgYDVQQKDA9ORVVDUkFDS19DTElFTlQx\n\
DzANBgNVBAsMBkNMSUVOVDEYMBYGA1UEAwwPbmV1Y3JhY2tfY2xpZW50MSIwIAYJ\n\
KoZIhvcNAQkBFhNjemQ2NjY2NjZAZ21haWwuY29tMIIBIjANBgkqhkiG9w0BAQEF\n\
AAOCAQ8AMIIBCgKCAQEArx3otIKLYnS3/eMPPV/RqYNIV6MQ9QYZn+WMleeVi+sr\n\
ANGo2eyGRcmHHTofZPA+bFQnunh+CwX2maQ2DxBxK0jSwUS4IFtXm2z0/eYS7NbV\n\
ehlEnJ/Zryklkuohy9tItNVGlnODHQqrqzO4UEcAd+tGxC4shuHtB+ghCt/qyTcm\n\
lrY3/2sDBp//J52I8wi0TRz6hYb1mbzavL2BMaGlmq9TITQ9XSYa+YTHREwvxYKQ\n\
HEFB8fcvrtkb5NZdtqWRsbLCoRUU1ZwDq85G1PRP763KUzgiRiPCKBY8GM05ePft\n\
xzPwDz9gTg2x2MUtBKMz9wsSKODxuhgtrxjoYIMcgwIDAQABMA0GCSqGSIb3DQEB\n\
CwUAA4IBAQBAehFa7NdP0CFf6hEZ5JvcdKYbyGFhcmufduTRfx7JFPL8IgYt56NA\n\
do02CnBxN5145G+I79bQoXcbw/QK9u12AvTHJ3twow9/4pQZiS+tKLao/UAuZem3\n\
EOxFNGHApOmOhDj/ypbU3nox4SdbZGzO0HXK4T/mCIMKTMevQAoBYtR8rPJY3V10\n\
McUpgtqWJGo2WqSB84+4RzzEP3ySHkAYh83xELv1imyevoNl128tjfagHYMLf1Yy\n\
q5gzlewghsN0T0D32ysswWHNI8abR4Gkq8pTsS6gGQf/6MV7ih6k0I9Lk3+nubFP\n\
R3/EQfUl+7lP5rg5hyi9ryPLgcPyjdNZ\n\
-----END CERTIFICATE-----";

const char* client_key = "-----BEGIN RSA PRIVATE KEY-----\n\
MIIEpQIBAAKCAQEArx3otIKLYnS3/eMPPV/RqYNIV6MQ9QYZn+WMleeVi+srANGo\n\
2eyGRcmHHTofZPA+bFQnunh+CwX2maQ2DxBxK0jSwUS4IFtXm2z0/eYS7NbVehlE\n\
nJ/Zryklkuohy9tItNVGlnODHQqrqzO4UEcAd+tGxC4shuHtB+ghCt/qyTcmlrY3\n\
/2sDBp//J52I8wi0TRz6hYb1mbzavL2BMaGlmq9TITQ9XSYa+YTHREwvxYKQHEFB\n\
8fcvrtkb5NZdtqWRsbLCoRUU1ZwDq85G1PRP763KUzgiRiPCKBY8GM05ePftxzPw\n\
Dz9gTg2x2MUtBKMz9wsSKODxuhgtrxjoYIMcgwIDAQABAoIBAQCW6+38dG3wYquX\n\
8jLWMFigAXc9oFoi8ijvd4ScRUTLm9gqEoe1mXV3e+IovEkGU09ZSYYb3XCzy2Xm\n\
sHGutfry1QZebv1JVa/BUjwq/jjdSaIeYtyRJ+sSXSH25jZOXSTiRNVbIR6kzud4\n\
21BET40FzQ3mUe0qKliAcfzYRHczBhmmTT/Nqdlwg3gPJMw9ub6ZPO1bpoIbTMjY\n\
x+yL0ybUzRajy4ol9q82pYg+OrhtlOZHxu8CKNrlSmsXB3WeJoii2GbW1CA2lynX\n\
UbDGF0zHf5Sq3ioWUSlPVJA04/IMEoIcGgG9w/3OHsrSpkWRo1sJ646wzfHaws64\n\
yvg0yXYBAoGBAN5CVKgVUn3EQ/X7O63CsoXIPi7E4+b/G1vERZbU8wJrQq3BjMOH\n\
DIrUaqMUaR1c/9kZv19YLpSCOU0kuGHYMhATtOYHdCUobLycS1o1e7Ftz+xZZn2A\n\
nhfJqRwybsSjGx+uIxhgn1y//ak//h9Upat63q4GR+2sctARdFkOMmwDAoGBAMmz\n\
fDr9rWoeEaEK9yOBdTowqP5QQsMtKOyHt21AFYCRHdIax13DyP1OhBZNAgjOpatl\n\
CV+pzUK8tC8DL3Lo/i+HYRY6j7VU/JXHBaXa0XyjL/dv24B5Sk+HJWWMJ7EESkMa\n\
oiuv3j70z1DBYg+nqibXiQD5KNOSRUM9Ksvk1OWBAoGBAIC/0+J6V4rbQrMfkfaa\n\
LmgpEzxseci6RtLl45n4Sb1A/SNCV5R8ittxKUTG8g08bkkJDT7iCjmlJiR1/QlQ\n\
/88oW0ZVUwajYDnyIARlrv2dKzEyqJhTBUGqY/gdjuUBxxiW+2WHs0k7Pmno1AC6\n\
PC4VwyD0Kn6Xwvqc4dUk0DiZAoGAEJUdBYyNkUffkynfn5+pD5uoiR/4HSu9TZTq\n\
06SNIADOPtPMyDysbYbi/SJfpHG41Rp9ZswDABWbirHsWLonC6lJphYycmoGShoO\n\
sNAIoQKZkvIzTaCrJ1apEGwxZzA/Gr05z/Wpg+uEj9MWSMs0BfV6YgYaASrpXtnM\n\
hu7NQgECgYEAg6s/toJ8fwGlU+o2GBJdU0d206OWZ+OvYf87OISVX9xnVkvoBPz6\n\
2Vo31PURShDzjd4QIJQq8Pw+4WGczoeWrtqhfV5V+52b3v8ii69fFc8daVBa8rke\n\
OLjDMAYBm2U0rEVJRGhH3G7zP31fe4v4LzpCOasozf1Vm0YXapQ6KlI=\n\
-----END RSA PRIVATE KEY-----";

int Https_Get(const char* domain, int port,const char* path, char* retBuffer, int* bufferLen)
{
return 0;
}

int Https_Post(const char* domain, int port, const char* path, const char* contentType, uint8_t* body, uint16_t bodyLen, char* retBuffer, int bufferLen)
{
    /*int32_t fdFile;
    int32_t retFile;
    uint32_t bufSize = 1024;
    uint8_t body[bufSize];

    fdFile = API_FS_Open(fileName, (FS_O_RDONLY | FS_O_CREAT), 0);
    if (fdFile < 0)
    {
        Trace(5, "Open file failed:%d", fdFile);
        return -1;
    }
    uint64_t bodyLen = API_FS_GetFileSize(fdFile);
    Trace(5, "bodyLen:%lld", bodyLen); */

    char *temp = OS_Malloc(2048);
    if (!temp)
    {
        Trace(2, "malloc fail");
        return -1;
    }
    snprintf(temp, 2048, "POST %s HTTP/1.1\r\nContent-Type: %s\r\nConnection: Keep-Alive\r\nHost: %s\r\nContent-Length: %d\r\n\r\n",
             path, contentType, domain, bodyLen);
    char *pData = temp;
    Trace(2,"%s",pData);

    int ret;
    SSL_Error_t error;
    SSL_Config_t config = {
        .caCert = ca_cert,
        .caCrl = NULL,
        .clientCert = client_cert,
        .clientKey = client_key,
        .clientKeyPasswd = NULL,
        .hostName = SSL_SERVER_HOST_NAME,
        .minVersion = SSL_VERSION_SSLv3,
        .maxVersion = SSL_VERSION_TLSv1_2,
        .verifyMode = SSL_VERIFY_MODE_NONE,
        .entropyCustom = "GPRS"};
    error = SSL_Init(&config);
    if (error != SSL_ERROR_NONE)
    {
        Trace(5, "ssl init error:%d", error);
        return error;
    }
    Trace(5, "ssl init success");

    error = SSL_Connect(&config, SSL_SERVER_ADDRESS, SSL_SERVER_PORT);
    if (error != SSL_ERROR_NONE)
    {
        Trace(5, "ssl connect error:%d", error);
        goto exit;
    }
    Trace(5, "ssl connect success");

    Trace(5, "write len:%d data:%s", strlen(pData), pData);
    ret = SSL_Write(&config, pData, strlen(pData), TIMEOUT_WRITE);
    if (ret <= 0)
    {
        error = ret;
        Trace(5, "ssl write fail:%d", error);
        goto exit0;
    }
    Trace(5, "ssl write header success");

    ret = SSL_Write(&config, body, bodyLen, TIMEOUT_WRITE);
    
    //读取服务器响应数据
    memset(retBuffer, 0, bufferLen);
    char cache[1024];
    memset(cache, 0, sizeof(cache));

    ret = SSL_Read(&config, cache, sizeof(cache), TIMEOUT_READ);
    while(ret>0)
    {
        Trace(5, "%s", cache);
        strcat(retBuffer,cache);
        ret = SSL_Read(&config, cache, sizeof(cache), TIMEOUT_READ);
    }

    Trace(5, "ssl read finish");
    error = 0;

exit0:
    Trace(5, "ssl close");
    SSL_Close(&config);
exit:
    Trace(5, "ssl destroy");
    SSL_Destroy(&config);
    //API_FS_Close(fdFile);
    return error;
}

int Https_PostBin(const char* domain, int port,const char* path,char* fileName, char* retBuffer, int bufferLen)
{
    int32_t fdFile;
    int32_t retFile;
    uint32_t bufSize = 1024;
    uint8_t body[bufSize];

    fdFile = API_FS_Open(fileName, (FS_O_RDONLY | FS_O_CREAT), 0);
    if (fdFile < 0)
    {
        Trace(5, "Open file failed:%d", fdFile);
        return -1;
    }
    uint64_t bodyLen = API_FS_GetFileSize(fdFile);
    Trace(5, "bodyLen:%lld", bodyLen); //7590

    char *temp = OS_Malloc(2048);
    if (!temp)
    {
        Trace(2, "malloc fail");
        return -1;
    }
    snprintf(temp, 2048, "POST %s HTTP/1.1\r\nContent-Type: application/octet-stream\r\nConnection: Keep-Alive\r\nHost: %s\r\nContent-Length: %d\r\n\r\n",
             path, domain, (uint32_t)bodyLen);
    char *pData = temp;
    Trace(2,"%s",pData);

    int ret;
    SSL_Error_t error;
    SSL_Config_t config = {
        .caCert = ca_cert,
        .caCrl = NULL,
        .clientCert = client_cert,
        .clientKey = client_key,
        .clientKeyPasswd = NULL,
        .hostName = SSL_SERVER_HOST_NAME,
        .minVersion = SSL_VERSION_SSLv3,
        .maxVersion = SSL_VERSION_TLSv1_2,
        .verifyMode = SSL_VERIFY_MODE_NONE,
        .entropyCustom = "GPRS"};
    error = SSL_Init(&config);
    if (error != SSL_ERROR_NONE)
    {
        Trace(5, "ssl init error:%d", error);
        return error;
    }
    Trace(5, "ssl init success");

    error = SSL_Connect(&config, SSL_SERVER_ADDRESS, SSL_SERVER_PORT);
    if (error != SSL_ERROR_NONE)
    {
        Trace(5, "ssl connect error:%d", error);
        goto exit;
    }
    Trace(5, "ssl connect success");

    Trace(5, "write len:%d data:%s", strlen(pData), pData);
    ret = SSL_Write(&config, pData, strlen(pData), 5000);
    if (ret <= 0)
    {
        error = ret;
        Trace(5, "ssl write fail:%d", error);
        goto exit0;
    }
    Trace(5, "ssl write header success");

    retFile = API_FS_Read(fdFile, (uint8_t *)body, sizeof(body));
    while (retFile > 0)
    {
        ret = SSL_Write(&config, body, sizeof(body), 5000);
        if (ret <= 0)
        {
            error = ret;
            Trace(5, "ssl write fail:%d", error);
            goto exit0;
        }
        Trace(5, "ssl write success");
        retFile = API_FS_Read(fdFile, body, bufSize);
    }

    //读取服务器响应数据
    memset(retBuffer, 0, bufferLen);
    char cache[1024];
    memset(cache, 0, sizeof(cache));

    ret = SSL_Read(&config, cache, sizeof(cache), 30000);
    while(ret>0)
    {
        Trace(5, "%s", cache);
        strcat(retBuffer,cache);
        ret = SSL_Read(&config, cache, sizeof(cache), 30000);
    }

    Trace(5, "ssl read finish");
    error = 0;

exit0:
    Trace(5, "ssl close");
    SSL_Close(&config);
exit:
    Trace(5, "ssl destroy");
    SSL_Destroy(&config);
    API_FS_Close(fdFile);
    return error;
}