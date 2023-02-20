#include "ibeelink_http.h"
#include "../main_app.h"
#include "../device_info.h"
#include "../linux_tool/busy_box.h"
#include <stdio.h>
#include <string.h>
extern void playerListAndConfig(char *rootStr);

/**
 * @brief 获取及处理配置和广告任务
 * @attention http底层接口内存问题
 * @attention 调用者不用关心具体如何处理
 * @attention 处理后的结果由playControl.c处理
 */
void handleConfigAndAD(char *url)
{
    void *request = ghttp_malloc();
    char *result = NULL;
    //int ret = ghttp_get_json(request,GET_AD_AND_CONFIG_URL,&result);
    if (ghttp_get_json(request,url,&result)>0)
    {
        printf("@@@@@@@@@@@@@@@@@[%s]\n",result);//拉到的配置和广告信息
        playerListAndConfig(result);
    }
    ghttp_get_free(request);
}

/**
 * @brief 回复获取配置后的处理结果
 * @attention 由于平台限制 只能回复成功
 * @param id 获取成功的id号
 */
void replyConfigResult(int id)
{
    void *request = ghttp_malloc();
    char *result = NULL;
    char postStr[100] = {0};
    sprintf(postStr,"{\"id\":%d,\"deviceId\":\"%s\"}",id,UnGetMD5DeviceId());
    ghttp_post_json(request,REPORT_CONFIG_URL,postStr,&result);
    ghttp_post_free(request);
}

/**
 * @brief 回复广告任务处理结果
 * @attention 包括成功、失败、删除等JSON数组
 * @param jsonStr 三个JSON数组组成的JSON字串
 * @example {\"success\":[380,381,333],\"failed\":[380,381,333],\"delete\":[380,381,333]}
 */
void replyADResult(char *jsonStr)
{
    void *request = ghttp_malloc();
    char *result = NULL;
    ghttp_post_json(request,REPORT_AD_URL,jsonStr,&result);
    ghttp_post_free(request);
}

/**
 * @brief 下载广告任务文件
 * 
 * @param downURL 下载地址
 * @param fileName 文件唯一名称
 */
void downloadTaskFile(char *downURL,char *fileName)
{
    /*处理URL中的空格问题 使用+号替换*/
    for (size_t i = 0; i < strlen(downURL); i++)
    {
        if (downURL[i]==' ')
        {
            downURL[i]='+';
        }
    }
    /*说明是新的广告任务 需要发起下载*/
    if(strstr(downURL,"https://"))
    {
        snprintf(downURL,256,"http://%s",downURL+8);
    }
    char tmp[512] = {0};
    snprintf(tmp,512,"%s/%s",USER_NLIST_ROOT,fileName);
    printf("base.mediaId[%s],downURL[%s]\n",tmp,downURL);
    /*开始使用http客户端下载任务文件*/
    if( 0 != http_download(downURL,tmp))
    {
        if( 0 != http_download(downURL,tmp))
        {
            return;
        }
    }
    call_system_cmd("sync");
    UnSleep_S(1);
}

/**
 * @brief 蜜连平台http注册接口
 * 
 */
#define IBEELINK_JSON "{\"location\":\"%s\",\"version\":\"%s\",\"signal\":%d,\"others\":%s}"
void ibeelink_register(char*json_str)//排除
{

    void *request = ghttp_malloc();
    char *result = NULL;

    char *url_post = (char *)malloc(1024);
    char *post_str = (char *)malloc(1024);
    memset(url_post,0,1024);
    memset(post_str,0,1024);

    sprintf(url_post,REGISTER_URL,UnGetDeviceId());
    sprintf(post_str,IBEELINK_JSON,UnGetLBSInfo(),MAIN_APP_VERSION, UnGetCsqValue(),json_str);
    ghttp_post_json(request,url_post,post_str,&result);
    printf("url_post[%s] post_str[%s] result[%s]\n",url_post,post_str,result);

	free(post_str);
	free(url_post);
    ghttp_post_free(request);
}
