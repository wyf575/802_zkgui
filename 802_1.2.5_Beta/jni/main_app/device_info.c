#include "main_app/AT_tool/user_AT.h"
#include "utils/Log.h"
#include "device_info.h"
#include "main_app/main_app.h"
#include "md5_tool/md5.h"

#define printf LOGD


/**
 * @brief 通用接口 获取设备IMEI
 *          注意：有阻塞操作
 * @return const char* 
 */
const char *UnGetDeviceId(void)
{
    static char imei[20] = {0};
    while (strlen(imei) < 15)
    {
        char buff_tmp[200] = {0};
        int failed_count = 3;
        while((--failed_count) && send_recv_AT_cmd("AT+CGSN\r\n","OK",buff_tmp,sizeof(buff_tmp))<=0)
        {
            printf("send_recv_AT_cmd[%s].................\n","AT+CGSN");
            sleep(1);
        }
        if (failed_count!=0)
        {
            sscanf(buff_tmp, "%*[^+]+CGSN\r\n%s\n", &imei);
        }
        printf("send_recv_AT_cmd imei[%s].................\n",imei);
    }
    return (const char *)imei;
}

const char *UnGetMD5DeviceId(void)
{
    static char md5_imei[50] = {0};
    if (0 == strlen(md5_imei))
    {
        CalculateMD5(UnGetDeviceId(), md5_imei, sizeof(md5_imei), 1);
    }
    return (const char *)md5_imei;
}

const char *UnGetICCId(void)
{
    static char iccid[30] = {0};
    if (strlen(iccid)<15)
    {
        int failed_count = 3;
        char buff_tmp[200] = {0};
        while((--failed_count) && send_recv_AT_cmd("AT+ICCID\r\n","+ICCID",buff_tmp,sizeof(buff_tmp))<=0)
        {
            printf("send_recv_AT_cmd[%s].................\n","AT+ICCID");
            sleep(1);
        }
        if (failed_count!=0 )
        {
            sscanf(buff_tmp, "%*[^ ] %s%*[^\n]", &iccid);
        }
        printf("send_recv_AT_cmd iccid[%s].................\n",iccid);
    }
    return (const char *)iccid;
}

int UnGetCsqValue(void)
{
    int rssi = 0, ber = 0;
    char buff_tmp[200] = {0};
    while(send_recv_AT_cmd("AT+CSQ\r\n","+CSQ",buff_tmp,sizeof(buff_tmp))<=0)
    {
        printf("send_recv_AT_cmd[%s].................\n","AT+CSQ");
        sleep(1);
    }
    sscanf(buff_tmp, "%*[^ ] %d,%d%*[^\n]", &rssi,&ber);
    printf("send_recv_AT_cmd rssi[%d].................\n",rssi);
    return rssi;
}

creg_AT_info getCregInfo(void)
{
    creg_AT_info reg = {0};
    static int flag = 0;
    char buff_tmp[200] = {0};
    if (0 == flag)
    {
        while(send_recv_AT_cmd("AT+CREG=2\r\n","OK",buff_tmp,sizeof(buff_tmp))<=0)
        {
            printf("send_recv_AT_cmd[%s].................\n","AT+CREG=2");
            sleep(1);
        }
        flag = 1;
    }
    memset(buff_tmp,0,sizeof(buff_tmp));
    while(send_recv_AT_cmd("AT+CREG?\r\n","OK",buff_tmp,sizeof(buff_tmp))<=0)
    {
        printf("send_recv_AT_cmd[%s].................\n","AT+CREG?");
        sleep(1);
    }
    sscanf(buff_tmp, "%*[^:]: %d,%d,\"%x\",\"%x\"%*", &reg.urcStatus,&reg.registerStat,&reg.lacInfo,&reg.ciInfo);
    return reg;
}

const char *UnGetLBSInfo(void)//排除
{
    creg_AT_info reg = {0};
    static char lbs_info[50] = {0};
    char buff_tmp[200] = {0};
    reg = getCregInfo();
    printf("urcStatus[%d],registerStat[%d],lacInfo[%ld],ciInfo[%ld]",reg.urcStatus,reg.registerStat,reg.lacInfo,reg.ciInfo);
    if (0 != reg.lacInfo)
    {
        snprintf(lbs_info, 50,"%d.%d.%d", reg.lacInfo, reg.ciInfo, UnGetCsqValue());
    }
    return (const char *)lbs_info;
}

int UnGetCsqLevel(void)
{
    int csq = UnGetCsqValue();
    return (csq/8+1);
}

#if NEED_DEVICE_SN
#include "main_app/ibeelink_app/ibeelink_http.h"
#include "main_app/cJSON_file/json_app.h"
#include "main_app/net_listener.h"
#include "main_app/debug_printf/debug_printf.h"
#include "restclient-cpp/restclient.h"

const char *UnGetDeviceSN(void)
{
    int failed = 0;
    while (NET_STATUS_OK != get_network_status())
    {
        debuglog_red("NET_STATUS_OK != get_network_status() failed[%d]\n",failed);
        if (failed>=3)
        {
            return NULL;
        }
        failed++;
        UnSleep_S(1);
    }
    static char deviceSN[10] = {0};
    if (strlen(deviceSN) < 6)
    {
        char url[256] = {0};
        char result[1024] = {0};
        snprintf(url, 256, SN_URL , UnGetMD5DeviceId()); 
        printf("===================url[%s]===================\n",url);
        // http_get_json(url,result,sizeof(result));
        RestClient::Response response = RestClient::get(url);
        if(response.code == 200){
            strncpy(result,response.body.c_str(),1024);
        }
        debuglog_red("===================result[%s]===================\n",result);
        cJSON *root = cJSON_Parse(result);
        cJSON *data = cJSON_GetObjectItem(root,"data");
        parse_json_table(data,J_String,"sn",deviceSN);
        cJSON_Delete(root);
    }
    return (const char *)deviceSN;
}

const char *get_popularize_str(void)
{
    static char tmp[256] = {0};
    const char *netSN = UnGetDeviceSN();
    if (NULL != netSN)
    {
        snprintf(tmp,256,POPULARIZE_URL,netSN);
    }
    return tmp;
}


#endif


//honestar karl.hong add 20210918
int UnIsDeviceWorking(void)
{
	int failed_count = 20;
	char buff_tmp[200] = {0};
	while((--failed_count) && send_recv_AT_cmd("AT\r\n","OK",buff_tmp,sizeof(buff_tmp))<=0)
	{
	    sleep(1);
	}
	
	if (failed_count!=0)
	{
		printf("4G module is working\n");
		return 0;
	}else{
		printf("4G module is not working\n");
		return -1;
	}
}
//honestar kar.hong add 20210918


#if 0
/*读取设备必要的信息*/
// debuglog_yellow("\n============================ 设备信息读取开始[%s] ========================\n",MAIN_APP_VERSION);
// printf("UnGetDeviceId[%s] \n",UnGetDeviceId());
// printf("UnGetICCId[%s] \n",UnGetICCId());
// printf("UnGetCsqValue[%d] \n",UnGetCsqValue());
// printf("UnGetLBSInfo[%s] \n",UnGetLBSInfo());
// printf("UnGetMD5DeviceId[%s] \n",UnGetMD5DeviceId());
// debuglog_yellow("\n============================ 设备信息读取结束 ========================\n");
#endif