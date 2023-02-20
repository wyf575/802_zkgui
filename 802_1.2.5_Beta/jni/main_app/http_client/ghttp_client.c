
#include <stdio.h>
#include <string.h>
#include "ghttp.h"
/*本文件日志输出开关*/
#if 0
#define debuglog printf
#else
#define debuglog(format,...)
#endif
/*
返回消息体长度
*/
int ghttp_get_json(void *request,const char *url,char **result)
{
    ghttp_status status;
    if(ghttp_set_uri((ghttp_request *)request, url) == -1)
       return -1;
    if(ghttp_set_type((ghttp_request *)request, ghttp_type_get) == -1)//get
    	 return -1;
    ghttp_prepare((ghttp_request *)request);
    status = ghttp_process((ghttp_request *)request);
    if(status == ghttp_error)
    	 return -1;
    debuglog("Status code -> %d\n", ghttp_status_code((ghttp_request *)request));
    *result = ghttp_get_body((ghttp_request *)request);
    int bytes_read = ghttp_get_body_len((ghttp_request *)request);
    debuglog("buf[%d]:%s\n",bytes_read,*result);
    return bytes_read;
}

int ghttp_post_json(void *request,const char *url,const char * post_str,char **result)
{
    ghttp_status status;
    if (ghttp_set_uri((ghttp_request *)request, url) == -1)
        return -1;
    if (ghttp_set_type((ghttp_request *)request, ghttp_type_post) == -1)//post
        return -1;
    ghttp_set_header((ghttp_request *)request, http_hdr_Content_Type,"application/json");
    // ghttp_set_sync(request, ghttp_sync); //set sync
    ghttp_set_body((ghttp_request *)request, post_str, strlen(post_str));
    ghttp_prepare((ghttp_request *)request);
    status = ghttp_process((ghttp_request *)request);
    if (status == ghttp_error)
        return -1;
    *result = ghttp_get_body((ghttp_request *)request);
    int bytes_read = ghttp_get_body_len((ghttp_request *)request);
    debuglog("buf[%d]:%s\n",bytes_read,*result);
    return bytes_read;
}

void *ghttp_malloc(void)
{
    ghttp_request *request = ghttp_request_new();
    return request;
}

void ghttp_get_free(void *request)
{
    ghttp_request_destroy((ghttp_request *)request);
}

void ghttp_post_free(void *request)
{
    ghttp_clean((ghttp_request *)request);
}


#if 0

    // void *request = ghttp_malloc();
    // char *result = NULL;
    // int ret = ghttp_get_json(request,TEST1,&result);
    // debuglog("result[%d][%s][%d]\n",ret,result,strlen(result));
    // ghttp_get_free(request);

    // char post_str[256] = {0};
    // sprintf(post_str, "{\"location\":\"%s\",\"version\":\"%s\",\"signal\":%d,\"others\":{\"netMode\":%d}}","12.34.56","V1.3.6", 26,4);
    // void *request = ghttp_malloc();
    // char *result = NULL;
    // int ret = ghttp_post_json(request,REGISTER_URL,post_str,&result);
    // debuglog("result[%d][%s][%d]\n",ret,result,strlen(result));
    // ghttp_post_free(request);


    // http_download(TEST3,"./xxxx");
#endif