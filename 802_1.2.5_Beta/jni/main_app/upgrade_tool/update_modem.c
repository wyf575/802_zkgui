#include "update_modem.h"
#include "baselib/WatchdogManager.h"
#include "restclient-cpp/restclient.h"
#include "../cJSON_file/json_app.h"
#include "../device_info.h"
#include "../http_client/ghttp_client.h"
#include "../linux_tool/busy_box.h"

#define OC_HTTP_URL_LEN 256
#define REQUST_URL "http://%s/api/version/upgrade/did?deviceId=%s&version=%s&type=%s"

static void ota_local_process(char *flag,char *path)
{
	static char busyFlag = 0;
	while (busyFlag)
	{
		UnSleep_S(3000);
	}
	busyFlag = 1;
	
	printf("XXXXXXXXXXXXXXXXXXXota_local_processXXXXXXXXXXXXXXXXXXXXXX[%s][%s]\n",flag,path);
	// WATCHDOGMANAGER->closeWatchdog();
	
	char tmp_cmd[256] = {0};
	snprintf(tmp_cmd,256,"echo \"%s\">/data/ota_path",path);
	printf("cmd[%s]\n",tmp_cmd);
	call_system_cmd(tmp_cmd);
	
	memset(tmp_cmd,0,sizeof(tmp_cmd));
	snprintf(tmp_cmd,256,"echo \"%s\">/data/ota_flag",flag);
	printf("cmd[%s]\n",tmp_cmd);
	call_system_cmd(tmp_cmd);

    call_system_cmd("sync");
	UnSleep_S(3000);
	busyFlag = 0;
}
static void ota_local_process_core(char *flag,char *path)
{
	static char busyFlag = 0;
	while (busyFlag)
	{
		UnSleep_S(3000);
	}
	busyFlag = 1;
	
	printf("XXXXXXXXXXXXXXXXXXXota_local_process_coreXXXXXXXXXXXXXXXXXXXXXX[%s][%s]\n",flag,path);
	// WATCHDOGMANAGER->closeWatchdog();
	
	char tmp_cmd[256] = {0};
	snprintf(tmp_cmd,256,"echo \"%s\">/data/ota_path",path);
	printf("cmd[%s]\n",tmp_cmd);
	call_system_cmd(tmp_cmd);
	
	memset(tmp_cmd,0,sizeof(tmp_cmd));
	snprintf(tmp_cmd,256,"echo \"%s\">/data/ota_flag",flag);
	printf("cmd[%s]\n",tmp_cmd);
	call_system_cmd(tmp_cmd);

    call_system_cmd("sync");
	UnSleep_S(3000);
	busyFlag = 0;
}
static void app_download_test_core(const char *url)
{
	printf("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC[%s]\n",url);
	if(strstr(url,"https://"))
	{
		snprintf(url,256,"http://%s",url+8);
	}
    /*开始使用http客户端下载任务文件*/
	if( 0 != http_download(url,"/data/ota.tar.gz"))
	{
		if( 0 != http_download(url,"/data/ota.tar.gz"))
		{
			return;
		}
	}
	call_system_cmd("sync");
	call_system_cmd("chmod a+x /data/ota.tar.gz");
	UnSleep_S(3);
	ota_local_process("3","/data/ota.tar.gz");
	printf("*****************app_download_test_core down\n");
}

static void app_download_test(const char *url)
{
	printf("UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU[%s]\n",url);
	if(strstr(url,"https://"))
	{
		snprintf(url,256,"http://%s",url+8);
	}
    /*开始使用http客户端下载任务文件*/
	if( 0 != http_download(url,"/data/SStarOta.bin.gz"))
	{
		if( 0 != http_download(url,"/data/SStarOta.bin.gz"))
		{
			return;
		}
	}
	call_system_cmd("sync");
	call_system_cmd("chmod a+x /data/SStarOta.bin.gz");
	UnSleep_S(3);
	ota_local_process("2","/data/SStarOta.bin.gz");
}


//"serverTime":1610525387039 
static unsigned int parse_receive_upgrade(char *mgs,char *download_url)
{
    cJSON *proot=NULL;
    cJSON *pobject=NULL;
    cJSON *pobject_data=NULL;
    unsigned int crc32_result = 0;
	if (0 == strlen(mgs))
	{
		return 0;
	}
    proot = cJSON_Parse(mgs);
    pobject = cJSON_GetObjectItem(proot,"data");
    if(pobject==NULL){
        printf("receive data has no data\n");
        cJSON_Delete(proot);
        return 0;
    }
    pobject_data = cJSON_GetObjectItem(pobject,"file");
    if(pobject_data==NULL){
        printf(" receive file has no data\n");
        cJSON_Delete(proot);
        return 0;
    }
    strcpy(download_url,pobject_data->valuestring);
    pobject_data = cJSON_GetObjectItem(pobject,"crc");
    crc32_result = pobject_data->valueint;
    cJSON_Delete(proot);
    return crc32_result;
}


/*
返回大于0的crc32校验值-需要下载的文件校验值
app/core 直接进行下载升级操作
mcu返回文件下载的地址 在外部进行操作
*/
int get_ibeelink_download(char *server,char *type)
{
	char *requst_url = (char *)malloc(OC_HTTP_URL_LEN);
	char *down_url = (char *)malloc(OC_HTTP_URL_LEN);
	memset(requst_url,0,OC_HTTP_URL_LEN);
	memset(down_url,0,OC_HTTP_URL_LEN);
	if (strstr(type,"app"))
	{
		sprintf(requst_url,REQUST_URL,server,UnGetMD5DeviceId(),MAIN_APP_VERSION,type);
	}

	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@app@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	void *request = ghttp_malloc();
    char *result = NULL;
    ghttp_get_json(request,requst_url,&result);
	printf("[http_get_method][%s][%s]\n",requst_url,result);
	int ret = parse_receive_upgrade(result,down_url);
	ghttp_get_free(request);

	if (ret>0)
	{
		if (strstr(type,"app"))
		{
			app_download_test(down_url);
		}
	}
	free(requst_url);
	free(down_url);
	return ret;
}
/*
返回大于0的crc32校验值-需要下载的文件校验值
app/core 直接进行下载升级操作
mcu返回文件下载的地址 在外部进行操作
*/
int get_ibeelink_download_core(char *server,char *type)
{
	char *requst_url = (char *)malloc(OC_HTTP_URL_LEN);
	char *down_url = (char *)malloc(OC_HTTP_URL_LEN);
	memset(requst_url,0,OC_HTTP_URL_LEN);
	memset(down_url,0,OC_HTTP_URL_LEN);
	if (strstr(type,"core"))
	{
		sprintf(requst_url,REQUST_URL,server,UnGetMD5DeviceId(),MAIN_APP_VERSION,type);
	}

	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@core@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	void *request = ghttp_malloc();
    char *result = NULL;
    ghttp_get_json(request,requst_url,&result);
	printf("[http_get_method][%s][%s]\n",requst_url,result);
	int ret = parse_receive_upgrade(result,down_url);
	ghttp_get_free(request);

	if (ret>0)
	{
		if (strstr(type,"core"))
		{
			app_download_test_core(down_url);
		}
	}
	free(requst_url);
	free(down_url);
	return ret;
}

/*
在mqtt连接线程中调用 并将结果作为参数
传入到用户的回调函数中
*/
int ibeelink_updata_function(char *server)
{
	if (get_ibeelink_download(server,"app")<=0)
	
	if (get_ibeelink_download_core(server,"core")<=0)
	{
		goto getfail;
	}
	return 1;
	
getfail:
	return 0;

}


