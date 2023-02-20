#include "update_mcu.h"

#if (USER_APP_TYPE==USER_APP_CARGO)

#include "../crc_tool/crc32.h"
#include "../hardware_tool/user_uart.h"
#include "../net_listener.h"
#include "../cJSON_file/json_app.h"
#include "../cJSON_file/cJSON.h"
#include "../message_queue/message_queue.h"
#include "../usr_file_tool/usr_file.h"
#include "../linux_tool/busy_box.h"
#include "../ibeelink_app/ibeelink_http.h"
#include "../player_task/playControl.h"

#define OC_HTTP_URL_LEN 256
// const char *cargo_mcu_file = "/data/up_app.bin";
const char *cargo_mcu_file = "/data/mcu.bin";
static char mcu_server[OC_HTTP_URL_LEN] = {0};
unsigned long whole_crc32 = 0;
static char sub_info[7][20] ={0} ;

static msg_queue_thread update_mcu_queue_id = {0};
static msg_queue_thread mcu_update_sem = {0};
static msg_queue_thread ibeelink_ready_sem = {0};

const char *GetSetMcuMinVersion(char *SetVer)
{
    static char min_ver[20] = {0};
    if (NULL != SetVer)
    {
        memset(min_ver,0,sizeof(min_ver));
        strcpy(min_ver,SetVer);
    }
    return (const char *)min_ver;
}

static uint8_t ibeelinkCS(uint8_t* data,uint32_t lenth)
{
    unsigned int sum = 0,i = 0;
    for ( i = 1; i < lenth-1; i++)
    {
        if (3 != i)
        {
            sum = sum + data[i];
        }
    }
    return ((~sum) & 0x7F);
}

/*
    @描述：生成通知MCU进入boot
    @输入：从站地址
    @返回：生成结果
*/
static uint32_t NotifyMCU(uint8_t addr,uint8_t *data)
{
    uint8_t tmp_data[] = {0xF0, 0xA1, 0xFF, 0x00, addr,0xB1, 0x00, 0xF1};
    tmp_data[3] = ibeelinkCS(tmp_data,sizeof(tmp_data));
    memcpy(data,tmp_data,sizeof(tmp_data));
    return sizeof(tmp_data);
}

/*
    @描述：生成单包升级数据
    @输入：从站地址 分包编号 包长 数据
    @返回：生成结果 len(output) = len(input)+14
*/
static uint32_t SingleUpdateData(uint8_t addr,uint8_t index,uint32_t lenth,uint8_t *input,uint8_t *output)
{
    CRC32_CTX ctx = {0};
    crc32_calculate(input, lenth,&ctx);
    printf("=================CRCCalculatValue: [ %lx ]\n", ctx.crc);
    if (0xFF == addr)
    {
        uint8_t tmp_data[] = {0xF0, 0xB1, 0xFF, 0x00, 0xFF, 0xBB, index,(lenth>>8),(lenth&0xFF)};
        memcpy(output,tmp_data,sizeof(tmp_data));
        
    }else
    {
        uint8_t tmp_data[] = {0xF0, 0xB1, 0xFF, 0x00, addr, 0xB3, index,(lenth>>8),(lenth&0xFF)};
        memcpy(output,tmp_data,sizeof(tmp_data));
    }
    memcpy(output+9,input,lenth);
    uint8_t tmp_data[] = {ctx.crc32[0],ctx.crc32[1],ctx.crc32[2],ctx.crc32[3],0xF1};
    memcpy(output+9+lenth,tmp_data,5);
    output[3] = ibeelinkCS(output,9+lenth+5);
    return (9+lenth+5);
}

/*
    @描述：生成通知整包CRC32
    @输入：从站地址
    @返回：生成结果
*/
static uint32_t NotifyWholeCRC32(uint8_t addr,unsigned long whole_crc32,uint8_t *data)
{
    uint8_t crc32[4] = {0};
    crc32[0] = whole_crc32>>24;
    crc32[1] = (whole_crc32&0xFF0000)>>16;
    crc32[2] = (whole_crc32&0xFF00)>>8;
    crc32[3] = whole_crc32&0xFF;
    printf("whole_crc32[%08X] crc32-0[%02X] crc32-1[%02X] crc32-2[%02X] crc32-3[%02X]",
    whole_crc32,crc32[0],crc32[1],crc32[2],crc32[3]);
    if (0xFF == addr)
    {
        uint8_t tmp_data[] = {0xF0, 0xA1, 0xFF, 0x00, 0xFF, 0xB7,crc32[0],crc32[1],crc32[2],crc32[3],0xF1};
        memcpy(data,tmp_data,sizeof(tmp_data));
    }else
    {
        uint8_t tmp_data[] = {0xF0, 0xA1, 0xFF, 0x00, addr, 0xB7,crc32[0],crc32[1],crc32[2],crc32[3],0xF1};
        memcpy(data,tmp_data,sizeof(tmp_data));
    }
    return 11;
}

/*
    @描述：生成查询升级结果指令
    @输入：从站地址
    @返回：生成结果
*/
static uint32_t CheckUpdataResult(uint8_t addr,uint8_t *data)
{
    uint8_t tmp_data[] = {0xF0, 0xA1, 0xFF, 0x00, addr,0xB5, 0x00, 0xF1};
    tmp_data[3] = ibeelinkCS(tmp_data,sizeof(tmp_data));
    memcpy(data,tmp_data,sizeof(tmp_data));
    return sizeof(tmp_data);
}

/*
    @描述：拉取MCU升级信息
    @输入：从站地址
    @返回：生成结果
*/
static unsigned int get_ibeelink_file_url(char *mgs,char *download_url)
{
    if (0 == strlen(mgs))
	{
		return 0;
	}
    cJSON *proot = cJSON_Parse(mgs);
    cJSON *pobject = cJSON_GetObjectItem(proot,"data");
    if(pobject==NULL){
        printf("pobject receive data has no data\n");
        cJSON_Delete(proot);
        return 0;
    }
    cJSON *pobject_data = cJSON_GetObjectItem(pobject,"file");
    if(pobject_data==NULL){
        printf("pobject_data receive file has no data\n");
        cJSON_Delete(proot);
        return 0;
    }
    strncpy(download_url,pobject_data->valuestring,OC_HTTP_URL_LEN);
    pobject_data = cJSON_GetObjectItem(pobject,"crc");
    whole_crc32 = pobject_data->valuedouble;
    printf("=========(crc32)=[%lu]\n",whole_crc32);
    cJSON_Delete(proot);
    return whole_crc32;
}

static bool uart_driver_deal(FILE *fp,uint8_t addr,int i,uint32_t read_lenth)
{
    bool ret = false;
    char *file_tmp = (char *)malloc(1024);
    char *send_data = (char *)malloc(1024);
    memset(file_tmp,0,1024);
    memset(send_data,0,1024);

    if(read_data_by_fp(fp,file_tmp,read_lenth) < 0)
    {
        printf("Cannot Read File:[%s][%s]\n",cargo_mcu_file,file_tmp);
        ret = false;
    }else
    {
        int lenth = SingleUpdateData(addr,i,read_lenth,file_tmp,send_data);
        if (0xFF == addr)
        {
            user_uart_send(0,send_data,lenth);
            UnSleep_MS(1000);
            ret = true;
        }else{
            uint8_t data_byte = 0x00;
            user_uart_send(0,send_data,lenth);
            uint8_t *msg = (uint8_t *)message_queue_timeout(&update_mcu_queue_id,10*2);
            if (NULL != msg)
            {
                data_byte = *msg;
                message_queue_message_free(&update_mcu_queue_id,msg);
                if (0xB4 == data_byte)
                {
                    free(file_tmp);
                    free(send_data);
                    ret =  true;
                }
            }
        }
    }
    free(file_tmp);
    free(send_data);
    return ret;
}

/*
    @描述：拆包发送升级数据
    @输入：从站地址 广/单播地址
    @返回：生成结果
*/

static int SendSingleDate(uint8_t addr)
{
    int	i = 0;
    uint32_t total = read_file_size_byte(cargo_mcu_file);
    FILE *fp = fopen(cargo_mcu_file, "r");
    if(fp == NULL)
    {
        printf("Cannot Open File:%s\n",cargo_mcu_file);
        return -1;
    }
    printf("total[%s][%d]===========================\n",cargo_mcu_file,total);
    for ( i = 0; i < total/512; i++)
    {
        printf("SendSingleDate512[%d]\n",i);
        if (false == uart_driver_deal(fp,addr,i,512))
        {
            fclose(fp);
            return -3;
        }
    }
    if (0 != total%512)
    {
        printf("SendSingleDate%d[%d]\n",total%512,i);
        if (false == uart_driver_deal(fp,addr,i,total%512))
        {
            fclose(fp);
            return -4;
        }
    }
    fclose(fp);
    return 0;
}

/*
    @描述：单独升级某个从站
    @输入：保留 暂未使用
    @返回：生成结果
*/
static bool update_single_addr(uint8_t addr)
{
    uint8_t data_byte = 0x00;
    uint8_t send_data[11] = {0};
    UnSleep_MS(300);
    SendSingleDate(addr);
    NotifyWholeCRC32(addr,whole_crc32,send_data);
    user_uart_send(0,send_data,sizeof(send_data));
    uint8_t *msg = (uint8_t *)message_queue_timeout(&update_mcu_queue_id,10*2);
    if (NULL != msg)
    {
        data_byte = *msg;
        message_queue_message_free(&update_mcu_queue_id,msg);
        if (0xB8 == data_byte)
        {
            return true;
        }
    }
    return false;
}

/*
    @描述：对外接口(在升级协程中)
    @输入：模块升级结果
    @返回：生成结果 释放信号量 开启升级
*/
int update_mcu_callback(char *server, int status)
{
    printf("[%s]module_4g upgrade status: %d",server,status);
    strcpy(mcu_server,server);
    if (0 == status)
    {
        uint8_t *tmp = (uint8_t *)message_queue_message_alloc_blocking(&mcu_update_sem);
        *tmp = 0;
        message_queue_write(&mcu_update_sem,tmp);

        uint8_t *msg = (uint8_t *)message_queue_read(&ibeelink_ready_sem);
        message_queue_message_free(&ibeelink_ready_sem,msg);
    }
    return 0;
}

/*
    @描述：升级数据交互(在业务协程中)
    @输入：升级过程数据
    @返回：无
*/
void update_mcu_recv(uint8_t data_byte)
{
    printf("uart_recv_deal[%02X]\n",data_byte);
    uint8_t *tmp = (uint8_t *)message_queue_message_alloc_blocking(&update_mcu_queue_id);
    *tmp = data_byte;
    message_queue_write(&update_mcu_queue_id,tmp);
}

/*
    @描述：广播升级从站
    @输入：从站表 在线程中调用
    @返回：生成结果 
*/
int mcu_download_callback(char *server, int status)
{
    if(access(cargo_mcu_file,F_OK) == 0)
    {
        printf("mcu_download_callback\n");
        int i = 0;
        UnSleep_MS(300);
        set_app_network_status(NET_STATUS_OK);
        /* 通知所有从站进入boot模式 */
        for ( i = 0; i < 7; i++)
        {
            if (strlen(sub_info[i])>0)
            {
                uint8_t send_data[8] = {0};
                NotifyMCU(i+1,send_data);
                user_uart_send(0,send_data,sizeof(send_data));
                uint8_t *msg = (uint8_t *)message_queue_timeout(&update_mcu_queue_id,10*2);
                if (NULL != msg)
                {
                    send_data[0] = *msg;
                    message_queue_message_free(&update_mcu_queue_id,msg);
                }
            }
        }
        UnSleep_MS(1000);
        /*广播发送升级数据 从站不回*/
        SendSingleDate(0xFF);
        UnSleep_MS(300);
        /*广播发送校验数据 从站不回*/
        uint8_t send_data[11] = {0};
        NotifyWholeCRC32(0xFF,whole_crc32,send_data);
        user_uart_send(0,send_data,sizeof(send_data));
        UnSleep_MS(300);
        /*查询所有从站升级结果*/
        for ( i = 0; i < 7; i++)
        {
            if (strlen(sub_info[i])>0)
            {
                uint8_t send_data[8] = {0};
                CheckUpdataResult(i+1,send_data);
                user_uart_send(0,send_data,sizeof(send_data));
                uint8_t *msg = (uint8_t *)message_queue_timeout(&update_mcu_queue_id,10*2);
                if (NULL != msg)
                {
                    send_data[0] = *msg;
                    message_queue_message_free(&update_mcu_queue_id,msg);
                }
            }
        }
        /*清除下载的升级文件*/
        user_remove_file(cargo_mcu_file);
        set_app_network_status(NET_STATUS_FAILE);
        return 1;
    }    
    return 0;
}

/*
    @描述：反复升级协程
    @输入：回调发布通知
    @返回：直接退出
*/
extern void RequestMcuInfo(char version[7][20]);
extern const char *UnGetMD5DeviceId(void);

void mcu_update_task(void *param)
{
    message_queue_init(&update_mcu_queue_id, sizeof(uint8_t), 15);
    message_queue_init(&mcu_update_sem, sizeof(uint8_t), 15);
    message_queue_init(&ibeelink_ready_sem, sizeof(uint8_t), 15);
    
    while (1)
    {
        int i = 0;
        
        char requst_url[OC_HTTP_URL_LEN] ={0};
        char download_url[OC_HTTP_URL_LEN] ={0};
        uint8_t *msg = (uint8_t *)message_queue_read(&mcu_update_sem);
        message_queue_message_free(&mcu_update_sem,msg);

        while (1)
        {
            printf("[thread]mcu_update_task!\n");
            /*先获取从站设备信息 得到请求升级的最小版本号*/
            RequestMcuInfo(sub_info);
            char min_ver[20] = "V256.256.256";
            unsigned long ver_total = 256256256;
            for ( i = 0; i < 7; i++)
            {
                if (strlen(sub_info[i])>0)
                {
                    int ver_1 = 0,ver_2 = 0,ver_3 = 0;
                    unsigned long tmp = 0;
                    sscanf(sub_info[i],"V%d.%d.%d",&ver_1,&ver_2,&ver_3);
                    tmp = ver_1*1000000+ver_2*1000+ver_3;
                    if (tmp<ver_total)
                    {
                        ver_total = tmp;
                        memset(min_ver,0,sizeof(min_ver));
                        strncpy(min_ver,sub_info[i],20);
                    }
                    printf("min_ver[%s]\n",min_ver);
                }
            }
            GetSetMcuMinVersion(min_ver);
            memset(requst_url,0,OC_HTTP_URL_LEN);
            memset(download_url,0,OC_HTTP_URL_LEN);
            snprintf(requst_url,OC_HTTP_URL_LEN,"http://%s/api/version/upgrade/did?deviceId=%s&version=%s&type=mcu",mcu_server,UnGetMD5DeviceId(),min_ver);
            
            void *request = ghttp_malloc();
            char *result = NULL;
            ghttp_get_json(request,requst_url,&result);
            printf("[http_get_method][%s][%s]\n",requst_url,result);
            unsigned int ret = get_ibeelink_file_url(result,download_url);
            ghttp_get_free(request);

            if(ret>0)
            {
                if(access(cargo_mcu_file,F_OK) == 0)
                {
                    user_remove_file(cargo_mcu_file);
                }
                printf("[http_get_method][%s]\n",download_url);

                /*开始使用http客户端下载任务文件*/
                if(strstr(download_url,"https://"))
                {
                    snprintf(download_url,256,"http://%s",download_url+8);
                }
                if( 0 != http_download(download_url,cargo_mcu_file))
                {
                    if( 0 != http_download(download_url,cargo_mcu_file))
                    {
                        continue;
                    }
                }
                call_system_cmd("sync");
                UnSleep_MS(1000);
                mcu_download_callback(download_url,1);
                UnSleep_MS(1000);
            }else
            {    
                uint8_t *tmp = (uint8_t *)message_queue_message_alloc_blocking(&ibeelink_ready_sem);
                *tmp = 0;
                message_queue_write(&ibeelink_ready_sem,tmp);
                break;
            }
        }
    }
}

#endif


