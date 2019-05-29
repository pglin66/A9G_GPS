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
 //����dns
#include "stdio.h"
#include "string.h"
#include "api_network.h"
#include "api_socket.h"
#include "time.h"//ʱ��
 //�绰
#include "api_call.h"
#include "api_audio.h"
//gpio
#include "api_hal_gpio.h"
#include "api_hal_pm.h"



#include "main.h"




#include <api_gps.h>

#include "api_hal_uart.h"







#include "api_info.h"
#include "assert.h"
#include "api_socket.h"
#include "api_network.h"
#include "api_lbs.h"

#include "api_sms.h"


#include "api_sim.h"


//gps
#include "gps_parse.h"
#include "math.h"
#include "gps.h"



#include "api_fs.h"
#include "api_charset.h"

#define AppMain_TASK_STACK_SIZE    (1024 * 2)
#define AppMain_TASK_PRIORITY      1 
HANDLE mainTaskHandle = NULL;
HANDLE otherTaskHandle = NULL;
HANDLE GPSTaskHandle = NULL;


bool isNetworkOk = false;//�����Ƿ�����
bool flag = false;
uint8_t iccid[21];
uint8_t imei[16];
char* token;


#define CONFIG_FILE_NAME "/test0.conf"
#define TF_CARD_TEST_FILE_NAME "/t/test_TF_card.txt"

#define TF_CARD_TEST_STRING    "12346test string, please open with utf-8! ��������������UTF-8���������ļ�Ŷ����Ȼ����������\r\n\r\n"

#define MAIN_TASK_STACK_SIZE    (2048 * 2)
#define MAIN_TASK_PRIORITY      0
#define MAIN_TASK_NAME          "Fs Test Task"



typedef struct {
	int a;
	int b;
	int c;
}Data_t;

//int Http_Pos(const char* domain, int port, const char* path, uint8_t* body, uint16_t bodyLen, char* retBuffer, int bufferLen)
//��ʼ����Ϣ
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

//��ʼ������1
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

//��ʼ��GPS

void GPSInit(VOID *pData) {
	GPS_Info_t* gpsInfo = Gps_GetInfo();//��ȡgps��Ϣ
	uint8_t buffer[300];

	//�ȴ�GPRSע�����
	//GPRSע������Ľ��̿��ܵ���GPS��Դ��ѹ�½���
	//����GPS������
	while (!flag)
	{
		Trace(1, "wait for gprs regiter complete");
		OS_Sleep(2000);
	}

	//open GPS hardware(UART2 open either) ��GPSӲ����UART2Ҳ���Դ򿪣�
	GPS_Init();
	GPS_Open(NULL);

	//wait for gps start up, or gps will not response command �ȴ�GPS����
	while (gpsInfo->rmc.latitude.value == 0) {
		OS_Sleep(1000);
	}

	// ����GPS NMEA������
	for (uint8_t i = 0; i < 5; ++i)
	{
		bool ret = GPS_SetOutputInterval(10000);
		Trace(1, "set gps ret:%d", ret);
		if (ret)
			break;
		OS_Sleep(1000);
	}

	// if(!GPS_ClearInfoInFlash()) //���gps��Ϣ
	//     Trace(1,"erase gps fail");

	// if(!GPS_SetQzssOutput(false))
	//     Trace(1,"enable qzss nmea output fail"); //����QZSS���NMEA

	// if(!GPS_SetSearchMode(true,false,true,false))//��������ģʽ
	//     Trace(1,"set search mode fail");

	// if(!GPS_SetSBASEnable(true))//����SBASģʽ
	//     Trace(1,"enable sbas fail");

	if (!GPS_GetVersion(buffer, 150)) {//��ȡgps�̼��汾
		Trace(1, "get gps firmware version fail");
	}
	else {
		Trace(1, "gps firmware version:%s", buffer);
	}
	// if(!GPS_SetFixMode(GPS_FIX_MODE_LOW_SPEED))//����fix ģʽ
	// Trace(1,"set fix mode fail");

	if (!GPS_SetOutputInterval(1000)) {//����gps������
		Trace(1, "set nmea output interval fail");

	}
	else {
		Trace(1, "init ok");
	}
	char pushgps[2048];
	char *pushgpsdata[256];
	while (1)
	{
		//show fix info
		uint8_t isFixed = gpsInfo->gsa[0].fix_type > gpsInfo->gsa[1].fix_type ? gpsInfo->gsa[0].fix_type : gpsInfo->gsa[1].fix_type;
		char* isFixedStr;
		if (isFixed == 2) {
			isFixedStr = "2D fix";
		}
		else if (isFixed == 3)
		{
			if (gpsInfo->gga.fix_quality == 1) {
				isFixedStr = "3D fix";
			}
			else if (gpsInfo->gga.fix_quality == 2) {
				isFixedStr = "3D/DGPS fix";
			}
		}
		else {
			isFixedStr = "no fix";
		}
		//convert unit ddmm.mmmm to degree(��)  ת����λ
		int temp = (int)(gpsInfo->rmc.latitude.value / gpsInfo->rmc.latitude.scale / 100);
		double latitude = temp + (double)(gpsInfo->rmc.latitude.value - temp * gpsInfo->rmc.latitude.scale * 100) / gpsInfo->rmc.latitude.scale / 60.0;
		temp = (int)(gpsInfo->rmc.longitude.value / gpsInfo->rmc.longitude.scale / 100);
		double longitude = temp + (double)(gpsInfo->rmc.longitude.value - temp * gpsInfo->rmc.longitude.scale * 100) / gpsInfo->rmc.longitude.scale / 60.0;
		//��γ�ȸ��Ƶ� http://www.gpsspg.com/maps.htm ���в鿴��ͼλ��
		snprintf(buffer, sizeof(buffer), "GPS fix mode:%d, BDS fix mode:%d, fix quality:%d, satellites tracked:%d, gps sates total:%d,gps1 sates total:%d, is fixed:%s, coordinate:WGS84, Latitude:%f, Longitude:%f, unit:degree,altitude:%f", gpsInfo->gsa[0].fix_type, gpsInfo->gsa[1].fix_type, gpsInfo->gga.fix_quality, gpsInfo->gga.satellites_tracked, gpsInfo->gsv[0].total_sats, gpsInfo->gsv[1].total_sats, isFixedStr, latitude, longitude, gpsInfo->gga.altitude);
		//�鿴��ص�ѹ  ��ص����ٷֱ�
		uint8_t percent;//����
		uint16_t v = PM_Voltage(&percent);//��ѹ
		Trace(1, "power:%d %d", v, percent);
		//����Gps		
		snprintf(pushgpsdata, sizeof(pushgpsdata), "token=%s&imei=%s&latitude=%f&longitude=%f&quality=%d&tracked=%d&altitude=%f&total=%d&speed=%f", token, imei,latitude, longitude, gpsInfo->gga.fix_quality, gpsInfo->gga.satellites_tracked, gpsInfo->gga.altitude, gpsInfo->gsv[0].total_sats,isFixed*1.0);
		//snprintf(requestPath,sizeof(buffer2),"/?id=%s&timestamp=%d&lat=%f&lon=%f&speed=%f&bearing=%.1f&altitude=%f&accuracy=%.1f&batt=%.1f",buffer, time(NULL), latitude, longitude, isFixed*1.0, 0.0, gpsInfo->gga.altitude, 0.0,percent*1.0);
		Http("plin.cc", 80, "/gms/push/gps", "POST", pushgpsdata, pushgps, sizeof(pushgps));
		Trace(1, "http response body:%s", http_body(pushgps));
		//show in tracer
		Trace(2, buffer);
		OS_Sleep(5000);
	}
}














bool SaveData(Data_t* data)
{
	int32_t fd;
	int32_t ret;
	bool retBool;
	uint8_t *unicodeName = NULL;
	uint32_t outlen = 0;

	retBool = LocalLanguage2UnicodeBigEndian(CONFIG_FILE_NAME, strlen(CONFIG_FILE_NAME) + 1, CHARSET_UTF_8, &unicodeName, &outlen);
	if (!retBool)
		return false;
	fd = API_FS_Open(unicodeName, FS_O_RDWR | FS_O_CREAT, 0);
	if (fd < 0)
	{
		Trace(1, "Open file failed:%d", fd);
		OS_Free(unicodeName);
		return false;
	}
	ret = API_FS_Write(fd, (uint8_t*)data, sizeof(Data_t));
	API_FS_Close(fd);
	OS_Free(unicodeName);
	if (ret <= 0)
		return false;
	return true;
}
bool ReadData(Data_t* data)
{
	int32_t fd;
	int32_t ret;
	bool retBool;
	uint8_t *unicodeName = NULL;
	uint32_t outlen = 0;

	retBool = LocalLanguage2UnicodeBigEndian(CONFIG_FILE_NAME, strlen(CONFIG_FILE_NAME) + 1, CHARSET_UTF_8, &unicodeName, &outlen);
	if (!retBool)
		return false;
	fd = API_FS_Open(unicodeName, (FS_O_RDONLY | FS_O_CREAT), 0);
	if (fd < 0)
	{
		OS_Free(unicodeName);
		return false;
	}
	ret = API_FS_Read(fd, (uint8_t*)data, sizeof(Data_t));
	Trace(1, "read ret:%d,sizeof(Data_t):%d", ret, sizeof(Data_t));
	API_FS_Close(fd);
	OS_Free(unicodeName);
	if (ret <= 0)
		return false;
	return true;
}

void FsInfoTest()
{
	API_FS_INFO fsInfo;
	int sizeUsed = 0, sizeTotal = 0;

	Trace(1, "Start Fs info test!");

	if (API_FS_GetFSInfo(FS_DEVICE_NAME_FLASH, &fsInfo) < 0)
	{
		Trace(1, "Get FS Flash device info fail!");
	}
	sizeUsed = fsInfo.usedSize;
	sizeTotal = fsInfo.totalSize;

	float mb = sizeTotal / 1024.0 / 1024.0;
	Trace(1, "T Flash used:%d Bytes, total size:%d Bytes(%d.%03d MB)", sizeUsed, sizeTotal, (int)mb, (int)((mb - (int)mb) * 1000));
}

void FsTestCase()
{
	Data_t data;
	Trace(1, "Start Fs read write test!");

	//read
	if (!ReadData(&data))
	{
		data.a = 0;
		data.b = 1;
		data.c = 2;
		Trace(1, "Read data fail");
	}
	else
	{
		if (data.a < 0)
			data.a = 0;
		if (data.b < 0)
			data.b = 1;
		if (data.c < 0)
			data.c = 2;
		Trace(1, "Read data success, a:%d, b:%d, c:%d", data.a, data.b, data.c);
	}
	//write
	++data.a;
	++data.b;
	++data.c;
	if (!SaveData(&data))
		Trace(1, "Save Data fail");
	else
		Trace(1, "Save data success, a:%d, b:%d, c:%d", data.a, data.b, data.c);
	//read
	memset(&data, 0, sizeof(Data_t));
	if (!ReadData(&data))
		Trace(1, "Read data fail");
	else
	{
		Trace(1, "Read data success, a:%d, b:%d, c:%d", data.a, data.b, data.c);
	}

}



bool FsTFTest()
{
	int32_t fd;
	int32_t ret;
	uint8_t *path = CONFIG_FILE_NAME;

	Trace(1, "Start Fs  TF card read write test!");

	fd = API_FS_Open(path, FS_O_RDWR | FS_O_CREAT, 0);
	if (fd < 0)
	{
		Trace(1, "Open file failed:%d", fd);
		return false;
	}
	ret = API_FS_Write(fd, (uint8_t*)TF_CARD_TEST_STRING, strlen(TF_CARD_TEST_STRING));
	API_FS_Close(fd);
	if (ret <= 0)
	{
		Trace(1, "write fail");
		return false;
	}
	Trace(1, "write success");
	return true;
}







void LoopTask(VOID *pData)
{
	UartInit();//��ʼ������
	SMSInit();//��ʼ����Ϣģ��
	while (!isNetworkOk)//�ȴ�����ע��ɹ���
	{
		OS_Sleep(100);
	}
	//��ȡimei
	memset(imei, 0, sizeof(imei));
	INFO_GetIMEI(imei);
	Trace(1, "IMEI:%s", imei);
	//��ȡsim��iccid
	memset(iccid, 0, sizeof(iccid));
	SIM_GetICCID(iccid);
	Trace(1, "ICCID:%s", iccid);

	
	Trace(1, "isNetworkOk success!");
	OS_Sleep(3000);
	FsInfoTest();
	FsTestCase();

	
	Trace(1, "fs test end");

	//ע���豸
	uint8_t login[1024];
	uint8_t* logindata[21];
	snprintf(logindata, sizeof(logindata), "imei=%s", imei);
	if (Http("plin.cc", 80, "/gms/push/login", "POST", logindata, login, sizeof(login))) {		
		token = http_body(login);
			
		if (!strcmp(token, "-1")) {
			Trace(1, "login -1");
		}
		else {
			Trace(1, "login success");
		
		}
		
	}
	else {
		Trace(1, "login fail");
	}
	
	Trace(1, "token %s",token);

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
	   

	//gpio
	GPIO_config_t gpioLedBlue = {
		.mode = GPIO_MODE_INPUT,//gpio ����ģʽ
		.pin = GPIO_PIN25,//gpio ����25
		.defaultLevel = GPIO_LEVEL_HIGH//����gpio�ߵ�ƽ
	};
	GPIO_Init(gpioLedBlue);//��ʼ��gpio



	
	OS_StartTask(GPSTaskHandle, NULL);

	while (1)
	{
		GPIO_LEVEL status = 0;//����Ϊ0
		GPIO_GetLevel(gpioLedBlue, &status);//��ȡgpio״̬
		if (status == 0)//���gpio״̬Ϊ0 ����绰
		{
			Trace(1, "status=0");//��ӡ����
			if (!CALL_Dial("15364195707"))//����
			{
				Trace(1, "CALLsuccess");//����绰�ɹ�

			}
		}
		Trace(3, "GPIO30 status:%d", status);

	;
		
		//��ȡ�ڴ�ʹ�����
		OS_Heap_Status_t heapStatus;
		OS_GetHeapUsageStatus(&heapStatus);

		Trace(1, "pOsHeapStatusp total:%d,used:%d", heapStatus.totalSize, heapStatus.usedSize);
		

		
		OS_Sleep(1000);
	}

	
}
void EventDispatch(API_Event_t* pEvent)
{

	switch (pEvent->id)
	{
	case API_EVENT_ID_POWER_ON://����

		break;
	case API_EVENT_ID_NO_SIMCARD: //��Чsim��

		Trace(10, "!!NO SIM CARD%d!!!!", pEvent->param1);
		break;
	case API_EVENT_ID_SYSTEM_READY:// ϵͳ׼������
		Trace(1, "system initialize complete");
		break;
	case API_EVENT_ID_NETWORK_REGISTERED_HOME://����ע��

	case API_EVENT_ID_NETWORK_REGISTERED_ROAMING://����ע��ɹ�
		Trace(1, "network register success");
		flag = true;		
		uint8_t status;
		Trace(2, "network register success");
		bool ret = Network_GetAttachStatus(&status);//��ȡ���Ż�վ״̬
		if (!ret)
			Trace(1, "get attach staus fail");
		Trace(1, "attach status:%d", status);
		if (status == 0)
		{
			ret = Network_StartAttach();//��������
			if (!ret)
			{
				Trace(1, "network attach fail");
			}
		}
		else
		{//���¸�������
			Network_PDP_Context_t context = {
				.apn = "cmnet",
				.userName = ""    ,
				.userPasswd = ""
			};
			Network_StartActive(context);
		}
		break;
	case API_EVENT_ID_NETWORK_ATTACHED://�������ӳɹ�
		Trace(1, "network attach success");
		Network_PDP_Context_t context = {
			.apn = "cmnet",
			.userName = ""    ,
			.userPasswd = ""
		};
		Network_StartActive(context);//ȥ��������
		break;

	case API_EVENT_ID_NETWORK_ACTIVATED://���缤��ɹ�
		Trace(1, "network activate success");
		isNetworkOk = true;//��ʶ���缤��ɹ�
		break;
	case API_EVENT_ID_DNS_SUCCESS://dns���ip��ַ�ɹ�
		Trace(1, "DNS get ip address from domain success(event),domain:%s,ip:%s", pEvent->pParam2, pEvent->pParam1);
		break;
	case API_EVENT_ID_DNS_ERROR://dns���ip��ַ����
		Trace(1, "DNS get ip address error(event)!!!");
		break;
	case API_EVENT_ID_CALL_DIAL://param1: isSuccess, param2:error code(CALL_Error_t)//���в���
		Trace(1, "Is dial success:%d, error code:%d", pEvent->param1, pEvent->param2);
		if (pEvent->param1)
			//isDialSuccess = true;
			break;
	case API_EVENT_ID_CALL_HANGUP:  //param1: is remote release call, param2:error code(CALL_Error_t)//���йҶ�
		Trace(1, "Hang up,is remote hang up:%d, error code:%d", pEvent->param1, pEvent->param2);
		break;
	case API_EVENT_ID_CALL_INCOMING:   //param1: number type, pParam1:number  //���绰��   pEvent->pParam1:�������    pEvent->param1:����
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
	case API_EVENT_ID_CALL_ANSWER://������ص�
		Trace(1, "CALL_Answer Callback");
		break;
	case API_EVENT_ID_CALL_DTMF:  //param1: key
		Trace(1, "received DTMF tone:%c", pEvent->param1);
		break;
	case API_EVENT_ID_GPS_UART_RECEIVED://���ܵ�gps����
		 Trace(1,"received GPS data,length:%d, data:%s,flag:%d",pEvent->param1,pEvent->pParam1,flag);
		GPS_Update(pEvent->pParam1, pEvent->param1);
		break;
	case API_EVENT_ID_SMS_SENT://������Ϣ�ɹ�
		Trace(2, "Send Message Success");
		break;
	case API_EVENT_ID_SMS_RECEIVED://�յ���Ϣ
		Trace(2, "received message");
		SMS_Encode_Type_t encodeType = pEvent->param1;
		uint32_t contentLength = pEvent->param2;//��Ϣ���ݳ���
		uint8_t* header = pEvent->pParam1;//header��Ϣ "+8615364195707",,"2018/12/25,17:23:33+08",145,17,0,2,"+8613800731505",145,10
		uint8_t* content = pEvent->pParam2;//��Ϣ����
		uint8_t tmp[500];


		Trace(2, "message header:%s", header);//��ӡheader ��Ϣ message header:"+8615364195707",,"2018/12/25,17:23:33+08",145,17,0,2,"+8613800731505",145,10
		Trace(2, "message content length:%d", contentLength);//��ӡ���ݳ��� message content length:10
		if (encodeType == SMS_ENCODE_TYPE_ASCII)//�ж϶�Ϣ����
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
		//�����յ��Ķ�Ϣ
		char pushsms[2048];
		char *pushsmsdata[256];
		snprintf(pushsmsdata,sizeof(pushsmsdata), "token=%s&imei=%s&iccid=%s&header=%s&body=%s",token, imei, iccid, header, tmp);
		Http("plin.cc", 80, "/gms/push/sms", "POST", pushsmsdata, pushsms, sizeof(pushsms));
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
	case API_EVENT_ID_UART_RECEIVED://��������
		if (pEvent->param1 == UART1)
		{
			
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
	GPSTaskHandle = OS_CreateTask(GPSInit, NULL, NULL, AppMain_TASK_STACK_SIZE, AppMain_TASK_PRIORITY, 1, 0, "GPS Task");
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