/*
 * @File  app_main.c
 * @Brief An example of SDK's mini system
 *
 * @Author: Neucrack
 * @Date: 2017-11-11 16:45:17
 * @Last Modified by: Neucrack
 * @Last Modified time: 2017-11-11 18:24:56
 */




#include "stdint.h"
#include "stdbool.h"
#include "api_os.h"
#include "api_event.h"
#include "api_debug.h"
 //网络dns
#include "stdio.h"
#include "string.h"
#include "api_network.h"
#include "api_socket.h"
#include "time.h"//时间
 //电话
#include "api_call.h"
#include "api_audio.h"
//gpio
#include "api_hal_gpio.h"
#include "api_hal_pm.h"


//gps
//#include "gps.h"
#include "main.h"




#include <api_gps.h>

#include "api_hal_uart.h"



#include "math.h"



#include "api_info.h"
#include "assert.h"
#include "api_socket.h"
#include "api_network.h"
#include "api_lbs.h"

#include "api_sms.h"


#include "api_sim.h"














#define AppMain_TASK_STACK_SIZE    (1024 * 2)
#define AppMain_TASK_PRIORITY      1 
HANDLE mainTaskHandle = NULL;
HANDLE otherTaskHandle = NULL;


bool isNetworkOk = false;//网络是否正常
bool flag = false;
bool isGpsOn = true;


//int Http_Pos(const char* domain, int port, const char* path, uint8_t* body, uint16_t bodyLen, char* retBuffer, int bufferLen)
//初始化短息
void SMSInit()
{
	if (!SMS_SetFormat(SMS_FORMAT_TEXT, SIM0))
	{
		Trace(1, "sms set format error");
		return;
	}
	SMS_Parameter_t smsParam = {
		.fo = 17 ,
		.vp = 167,
		.pid = 0  ,
		.dcs = 8  ,//0:English 7bit, 4:English 8 bit, 8:Unicode 2 Bytes
	};
	if (!SMS_SetParameter(&smsParam, SIM0))
	{
		Trace(1, "sms set parameter error");
		return;
	}
	if (!SMS_SetNewMessageStorage(SMS_STORAGE_SIM_CARD))
	{
		Trace(1, "sms set message storage fail");
		return;
	}
}

void UartInit()
{
	UART_Config_t config = {
		.baudRate = UART_BAUD_RATE_115200,
		.dataBits = UART_DATA_BITS_8,
		.stopBits = UART_STOP_BITS_1,
		.parity = UART_PARITY_NONE,
		.rxCallback = NULL,
	};
	UART_Init(UART1, config);
}




void LoopTask(VOID *pData)
{


	UartInit();
	SMSInit();



	while (!isNetworkOk)//循环到网络注册成功！
	{
		OS_Sleep(100);
	}
	//post();
	Trace(1, "isNetworkOk success!");



	while (1)
	{
		if (isGpsOn)
		{
		
		}

		OS_Sleep(5000);
	}

	/*




	uint8_t addr[32];
	uint8_t temp;
	SMS_Server_Center_Info_t sca;
	sca.addr = addr;
	SMS_GetServerCenterInfo(&sca);
	Trace(1, "server center address:%s,type:%d", sca.addr, sca.addrType);
	temp = sca.addr[strlen(sca.addr) - 1];
	sca.addr[strlen(sca.addr) - 1] = '0';
	if (!SMS_SetServerCenterInfo(&sca))
		Trace(1, "SMS_SetServerCenterInfo fail");
	else
		Trace(1, "SMS_SetServerCenterInfo success");
	SMS_GetServerCenterInfo(&sca);
	Trace(1, "server center address:%s,type:%d", sca.addr, sca.addrType);
	sca.addr[strlen(sca.addr) - 1] = temp;
	if (!SMS_SetServerCenterInfo(&sca))
		Trace(1, "SMS_SetServerCenterInfo fail");
	else
		Trace(1, "SMS_SetServerCenterInfo success");

	SMS_Storage_Info_t storageInfo;
	SMS_GetStorageInfo(&storageInfo, SMS_STORAGE_SIM_CARD);
	Trace(1, "sms storage sim card info, used:%d,total:%d", storageInfo.used, storageInfo.total);
	SMS_GetStorageInfo(&storageInfo, SMS_STORAGE_FLASH);
	Trace(1, "sms storage flash info, used:%d,total:%d", storageInfo.used, storageInfo.total);
	if (!SMS_DeleteMessage(10, SMS_STATUS_ALL, SMS_STORAGE_SIM_CARD))
		Trace(1, "delete sms fail");
	else
		Trace(1, "delete sms success");
	SMS_ListMessageRequst(SMS_STATUS_ALL, SMS_STORAGE_SIM_CARD);
	


	//char buffer[2048];

	//if (Http("plin.cc", 80, "/aaa.php","GET", NULL, buffer,sizeof(buffer)) < 0)
	//{
	//	Trace(1, "http get fail");
	//}
	//else
	//{
	//	Trace(1, "http get success,ret:%s", buffer);
	//	char* index0 = strstr(buffer,"\r\n\r\n");
	//	char temp = index0[4];
	//	index0[4] = '\0';
	//	Trace(1, "http response header:%s", buffer);
	//	index0[4] = temp;
	//	index0 = index0 + 4;
	//	Trace(1, "http response body:%s", index0);
	//
	//}
	
     

	//gpio
	GPIO_config_t gpioLedBlue = {
		.mode = GPIO_MODE_INPUT,//gpio 输入模式
		.pin = GPIO_PIN25,//gpio 引脚25
		.defaultLevel = GPIO_LEVEL_HIGH//设置gpio高电平
	};
	GPIO_Init(gpioLedBlue);//初始化gpio

	uint8_t iccid[21];
	uint8_t imei[16];




	while (1)
	{
		GPIO_LEVEL status = 0;//设置为0
		GPIO_GetLevel(gpioLedBlue, &status);//获取gpio状态
		if (status == 0)//如果gpio状态为0 拨打电话
		{
			Trace(1, "status=0");//打印调试
			if (!CALL_Dial("15364195707"))//拨号
			{
				Trace(1, "CALLsuccess");//拨打电话成功

			}
		}
		Trace(3, "GPIO30 status:%d", status);

	;	//获取imei
		memset(imei, 0, sizeof(imei));
		INFO_GetIMEI(imei);
		Trace(1, "IMEI:%s", imei);
		//获取sim卡iccid
		memset(iccid, 0, sizeof(iccid));
        SIM_GetICCID(iccid);		
        Trace(1,"ICCID:%s",iccid);
		
		//获取内存使用情况
		OS_Heap_Status_t heapStatus;
		OS_GetHeapUsageStatus(&heapStatus);

		Trace(1, "pOsHeapStatusp total:%d,used:%d", heapStatus.totalSize, heapStatus.usedSize);
		
			//
		//}
		
		OS_Sleep(1000);
	}
	*/

}
void EventDispatch(API_Event_t* pEvent)
{

	switch (pEvent->id)
	{
	case API_EVENT_ID_POWER_ON://启动

		break;
	case API_EVENT_ID_NO_SIMCARD: //无效sim卡

		Trace(10, "!!NO SIM CARD%d!!!!", pEvent->param1);

		break;

	case API_EVENT_ID_SYSTEM_READY:// 系统准备就绪
		Trace(1, "system initialize complete");
		break;

	case API_EVENT_ID_NETWORK_REGISTERED_HOME://网络注册

	case API_EVENT_ID_NETWORK_REGISTERED_ROAMING://网络注册成功
		Trace(1, "network register success");
		flag = true;
		//if (!CALL_Dial("15364195707"))//拨号
		//{
			//Trace(1, "make a call failed");
			//return;
		//}
		//flag = 1;
		//Network_StartAttach();//连接网络
		uint8_t status;
		Trace(2, "network register success");
		bool ret = Network_GetAttachStatus(&status);//获取附着基站状态
		if (!ret)
			Trace(1, "get attach staus fail");
		Trace(1, "attach status:%d", status);
		if (status == 0)
		{
			ret = Network_StartAttach();//连接网络
			if (!ret)
			{
				Trace(1, "network attach fail");
			}
		}
		else
		{//重新附着网络
			Network_PDP_Context_t context = {
				.apn = "cmnet",
				.userName = ""    ,
				.userPasswd = ""
			};
			Network_StartActive(context);
		}
		break;

	case API_EVENT_ID_NETWORK_ATTACHED://网络连接成功
		Trace(1, "network attach success");
		Network_PDP_Context_t context = {
			.apn = "cmnet",
			.userName = ""    ,
			.userPasswd = ""
		};
		Network_StartActive(context);//去附着网络
		break;

	case API_EVENT_ID_NETWORK_ACTIVATED://网络激活成功
		Trace(1, "network activate success");
		isNetworkOk = true;//标识网络激活成功
		break;

	case API_EVENT_ID_DNS_SUCCESS://dns获得ip地址成功
		Trace(1, "DNS get ip address from domain success(event),domain:%s,ip:%s", pEvent->pParam2, pEvent->pParam1);
		break;

	case API_EVENT_ID_DNS_ERROR://dns获得ip地址错误
		Trace(1, "DNS get ip address error(event)!!!");
		break;

	case API_EVENT_ID_CALL_DIAL://param1: isSuccess, param2:error code(CALL_Error_t)//呼叫拨号
		Trace(1, "Is dial success:%d, error code:%d", pEvent->param1, pEvent->param2);
		if (pEvent->param1)
			//isDialSuccess = true;
			break;
	case API_EVENT_ID_CALL_HANGUP:  //param1: is remote release call, param2:error code(CALL_Error_t)//呼叫挂断
		Trace(1, "Hang up,is remote hang up:%d, error code:%d", pEvent->param1, pEvent->param2);
		break;
	case API_EVENT_ID_CALL_INCOMING:   //param1: number type, pParam1:number  //来电话啦   pEvent->pParam1:来电号码    pEvent->param1:类型
		Trace(1, "Receive a call, number:%s, number type:%d", pEvent->pParam1, pEvent->param1);

		OS_Sleep(5000);
		if (!CALL_Answer()) {
			Trace(1, "CALL_Answer fail");
		}
		else
		{
			Trace(1, "CALL_Answer success");
		}
		break;
	case API_EVENT_ID_CALL_ANSWER://接听后回调

		Trace(1, "CALL_Answer Callback");

		break;
	case API_EVENT_ID_CALL_DTMF:  //param1: key
		Trace(1, "received DTMF tone:%c", pEvent->param1);
		break;


	case API_EVENT_ID_GPS_UART_RECEIVED://接受到gps数据
		 Trace(1,"received GPS data,length:%d, data:%s,flag:%d",pEvent->param1,pEvent->pParam1,flag);
		//GPS_Update(pEvent->pParam1, pEvent->param1);
		break;
	case API_EVENT_ID_SMS_SENT://发生短息成功
		Trace(2, "Send Message Success");
		break;
	case API_EVENT_ID_SMS_RECEIVED://收到短息
		Trace(2, "received message");
		SMS_Encode_Type_t encodeType = pEvent->param1;
		uint32_t contentLength = pEvent->param2;//短息内容长度
		uint8_t* header = pEvent->pParam1;//header信息 "+8615364195707",,"2018/12/25,17:23:33+08",145,17,0,2,"+8613800731505",145,10
		uint8_t* content = pEvent->pParam2;//短息内容
		uint8_t tmp[500];


		Trace(2, "message header:%s", header);//打印header 信息 message header:"+8615364195707",,"2018/12/25,17:23:33+08",145,17,0,2,"+8613800731505",145,10
		Trace(2, "message content length:%d", contentLength);//打印内容长度 message content length:10
		if (encodeType == SMS_ENCODE_TYPE_ASCII)//判断短息类型
		{
			Trace(2, "message content:%s", content);
			//UART_Write(UART1,content,contentLength);
		}
		else
		{

			memset(tmp, 0, 500);
			for (int i = 0; i < contentLength; i += 2)
				sprintf(tmp + strlen(tmp), "\\u%02x%02x", content[i], content[i + 1]);
			Trace(2, "message content(unicode):%s", tmp);//you can copy this string to http://tool.chinaz.com/tools/unicode.aspx and display as Chinese
			/*uint8_t* gbk = NULL;
			uint32_t gbkLen = 0;


			if(!SMS_Unicode2LocalLanguage(content,contentLength,CHARSET_CP936,&gbk,&gbkLen))
				Trace(10,"convert unicode to GBK fail!");
			else
			{
				memset(tmp,0,500);
				for(int i=0;i<gbkLen;i+=2)
					sprintf(tmp+strlen(tmp),"%02x%02x ",gbk[i],gbk[i+1]);
				Trace(2,"message content(GBK):%s",tmp);//you can copy this string to http://m.3158bbs.com/tool-54.html# and display as Chinese
				//UART_Write(UART1,gbk,gbkLen);//use serial tool that support GBK decode if have Chinese, eg: https://github.com/Neutree/COMTool
			}
			OS_Free(gbk);

			*/

		}
		//推送收到的短息
		char pushsms[2048];
		char *pushsmsdata[200];
		sprintf(pushsmsdata, "header=%s&body=%s", header, tmp);
		Http("plin.cc", 80, "/gms/push/sms", "POST", pushsmsdata, pushsms, strlen(pushsms));
		Trace(1, "http response body:%s", http_body(pushsms));

		//char pushsms[2048];
		//char *pushsmsdata;
		//sprintf(pushsmsdata, "{header:'%s',body:'%s'}", header, tmp);
		//Http("plin.cc", 80, "/gms/push/sms", "POST", '{a:1}', pushsms, sizeof(pushsms));
		//Trace(1, "http response body:%s", http_body(pushsms));

		break;
	case API_EVENT_ID_SMS_LIST_MESSAGE:
	{
		SMS_Message_Info_t* messageInfo = (SMS_Message_Info_t*)pEvent->pParam1;
		Trace(1, "message header index:%d,status:%d,number type:%d,number:%s,time:\"%u/%02u/%02u,%02u:%02u:%02u+%02d\"", messageInfo->index, messageInfo->status,
			messageInfo->phoneNumberType, messageInfo->phoneNumber,
			messageInfo->time.year, messageInfo->time.month, messageInfo->time.day,
			messageInfo->time.hour, messageInfo->time.minute, messageInfo->time.second,
			messageInfo->time.timeZone);
		Trace(1, "message content len:%d,data:%s", messageInfo->dataLen, messageInfo->data);
		UART_Write(UART1, messageInfo->data, messageInfo->dataLen);//use serial tool that support GBK decode if have Chinese, eg: https://github.com/Neutree/COMTool
		UART_Write(UART1, "\r\n\r\n", 4);
		//need to free data here
		OS_Free(messageInfo->data);
		break;
	}
	case API_EVENT_ID_SMS_ERROR:
		Trace(10, "SMS error occured! cause:%d", pEvent->param1);
	case API_EVENT_ID_UART_RECEIVED://串口数据
		if (pEvent->param1 == UART1)
		{
			uint8_t data[pEvent->param2 + 1];
			data[pEvent->param2] = 0;
			memcpy(data, pEvent->pParam1, pEvent->param2);
			Trace(1, "uart received data,length:%d,data:%s", pEvent->param2, data);
			if (strcmp(data, "close") == 0)
			{
				Trace(1, "close gps");
				GPS_Close();
				isGpsOn = false;
			}
			else if (strcmp(data, "open") == 0)
			{
				Trace(1, "open gps");
				GPS_Open(NULL);
				isGpsOn = true;
			}
		}
		break;
	default:
		break;
	}
}


void AppMainTask(VOID *pData)
{
	API_Event_t* event = NULL;

	otherTaskHandle = OS_CreateTask(LoopTask,
		NULL, NULL, AppMain_TASK_STACK_SIZE, AppMain_TASK_PRIORITY, 0, 0, "ohter Task");

	while (1)
	{
		if (OS_WaitEvent(mainTaskHandle, &event, OS_TIME_OUT_WAIT_FOREVER))
		{
			EventDispatch(event);
			OS_Free(event->pParam1);
			OS_Free(event->pParam2);
			OS_Free(event);
		}
	}
}



void app_Main(void)
{
	mainTaskHandle = OS_CreateTask(AppMainTask,
		NULL, NULL, AppMain_TASK_STACK_SIZE, AppMain_TASK_PRIORITY, 0, 0, "init Task");
	OS_SetUserMainHandle(&mainTaskHandle);





}