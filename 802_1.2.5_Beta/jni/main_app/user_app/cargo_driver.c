#include "cargo_driver.h"
#if (USER_APP_TYPE==USER_APP_CARGO)
#include "main_app/upgrade_tool/update_mcu.h"
#include "user_app.h"

extern void ibeelink_register(char*json_str);

// #include "update_mcu.h"
typedef unsigned char uint8_t;
/*框架相关全局静态变量*/
static msg_queue_thread uart2driver_queue_id = {0};

static char sub_version[7][20] ={0} ;
static uint8_t sub_addr = 0;
static uint8_t sub_count = 0;

typedef struct 
{
    const char *str;
    bool value;
}status_table_t;

static status_table_t status_table[7] = {
    {"HAVE_READ",true},
    {"SUCCESS",true},
    {"FAILED",false},
    {"REJECT",false},
    {"CHECK_ERROR",false},
    {"FORMAT_ERROR",false},
    {NULL,false},
};
static uint8_t match_index(uint8_t num)
{
	switch (num)
	{
	case 0x02:return 0;
	case 0x03:return 1;
	case 0x04:return 2;
	case 0x05:return 3;
	case 0x06:return 4;
	case 0x07:return 5;
	}
    return 6;
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

static bool check_data_legality(uint8_t *data,uint32_t lenth)
{
    if (0xF0 == data[0] && 0xF1 == data[lenth-1] && data[3] == ibeelinkCS(data,lenth))
    {
        return true;
    }
    return false;
}

static uint8_t UniqueFlag(void)
{
    static uint8_t unique_value = 0;
    if (0xFF == unique_value)
    {
        unique_value = 0;
    }
    unique_value = unique_value + 1;
    return unique_value;
}


/**
 * @brief 指定货道出货
 * 
 * @param digital 
 * @param data 
 * @return uint32_t 
 */
extern unsigned char getSetDebugTime(int setTimeS);

static uint32_t CargoDigital(uint8_t digital,uint8_t *data)
{
    uint8_t tmp_data[] = {0xF0, 0x01, UniqueFlag(), 0x00, digital,0x01, getSetDebugTime(0), 0xF1};
    tmp_data[3] = ibeelinkCS(tmp_data,sizeof(tmp_data));
    memcpy(data,tmp_data,sizeof(tmp_data));
    return sizeof(tmp_data);
}

/**
 * @brief 一键补货
 * 
 * @param addr 
 * @param data 
 * @return uint32_t 
 */
static uint32_t Replenishment(uint8_t addr,uint8_t *data)
{
    uint8_t tmp_data[] = {0xF0, 0x05, UniqueFlag(), 0x00, addr,0x01, 0x00, 0xF1};
    tmp_data[3] = ibeelinkCS(tmp_data,sizeof(tmp_data));
    memcpy(data,tmp_data,sizeof(tmp_data));
    return sizeof(tmp_data);
}

static void ReplyMcuCmd(uint8_t *tmp_data,uint32_t lenth)
{
    tmp_data[5] = 0x09;
    tmp_data[3] = ibeelinkCS(tmp_data,lenth);
}


void RequestMcuInfo(char version[7][20])
{
    cJSON *json_table = cJSON_CreateObject();
    memset(sub_version,0,sizeof(sub_version));
    UnSleep_MS(3000);
    for ( sub_addr = 1; sub_addr < 8; sub_addr++)
    {
        uint8_t data_byte = 0x00;
        uint8_t tmp_data[] = {0xF0, 0x06, UniqueFlag(), 0x00, sub_addr,0x01, 0x00, 0xF1};
        tmp_data[3] = ibeelinkCS(tmp_data,sizeof(tmp_data));
        user_uart_send(0, tmp_data, sizeof(tmp_data));
        uint8_t *msg = (uint8_t *)message_queue_timeout(&uart2driver_queue_id,10);
        if (NULL != msg)
        {
            data_byte = *msg;
            message_queue_message_free(&uart2driver_queue_id,msg);
        }
        if (strlen(sub_version[sub_addr-1])>0)
        {
            char tmp_str[20] = {0};
            snprintf(tmp_str,20,"sub_num[%d]",sub_addr);
            create_json_table(json_table,J_String,tmp_str,sub_version[sub_addr-1]);
            sub_count = sub_count +1;
        }
        UnSleep_MS(100);
    }
    char *json_str = cJSON_PrintUnformatted(json_table);
    while (NET_STATUS_OK != get_network_status())
    {
        UnSleep_MS(500);
    }
    printf("@@@@@@@@@@@@@@[%s]\n",json_str);
    ibeelink_register(json_str);
    free(json_str);
    cJSON_Delete(json_table);
    if (NULL != version)
    {
        memcpy(version,sub_version,sizeof(sub_version));
    }
}

static void ParseMcuInfo(uint8_t *data)
{
    sprintf(sub_version[sub_addr-1],"V%d.%d.%d",data[6],data[7],data[8]);
}

static bool uart_driver_deal(uint8_t *data,uint32_t lenth,post_back_t *post_back)
{
    uint8_t i = 0;
    for ( i = 0; i < 5; i++)
    {
        uint8_t data_byte = 0x00;
        user_uart_send(0, data, lenth);
        uint8_t *msg = (uint8_t *)message_queue_timeout(&uart2driver_queue_id,10*2);
        if (NULL != msg)
        {
            data_byte = *msg;
            message_queue_message_free(&uart2driver_queue_id,msg);
            if (!strcmp(status_table[match_index(data_byte)].str,"HAVE_READ"))
            {
                data_byte = 0x00;
                uint8_t *msg2 = (uint8_t *)message_queue_timeout(&uart2driver_queue_id,10*25);
                if (NULL != msg2)
                {
                    data_byte = *msg2;
                    message_queue_message_free(&uart2driver_queue_id,msg2);

                    post_back->result = status_table[match_index(data_byte)].value;
                    strcpy(post_back->debug,status_table[match_index(data_byte)].str);
                    ReplyMcuCmd(data,lenth);
                    user_uart_send(0, data, lenth);
                    return true;
                }
                post_back->result = false;
                strcpy(post_back->debug,"WAIT_FOR_STATUS");
                return false;
            }
        }
    }
    post_back->result = false;
    strcpy(post_back->debug,"WAIT_FOR_READ");
    return false;
}
extern void update_mcu_recv(uint8_t data_byte);

void uart_receive_thread(void *param)
{
    uint8_t *data = (uint8_t *)malloc(1024);
    while (1)
    {
        printf("[thread]uart_receive_thread!\n");
        memset(data,0,1024);
        int i = 0;
        int lenth = user_uart_receive(data,1023);
        for (i = 0; i < lenth; i++)
        {
            printf("r[%d][%02X] ", i,data[i]);
        }
        if (uart2driver_queue_id.max_depth > 0)
        {
            post_back_t post_back = {0};
            if(true == check_data_legality(data,lenth))
            {
                uint8_t data_byte = 0x00;
                uint8_t *tmp = NULL;//Warning: jump to case label [-fpermissive] case分支不要定义变量
                switch (data[1])
                {
                case 0x01://出货
                case 0x05://格子柜补货
                    tmp = (uint8_t *)message_queue_message_alloc_blocking(&uart2driver_queue_id);
                    *tmp = data[5];
                    message_queue_write(&uart2driver_queue_id,tmp);
                    break;
                case 0x06://轮询从站返回
                    ParseMcuInfo(data);
                    tmp = (uint8_t *)message_queue_message_alloc_blocking(&uart2driver_queue_id);
                    *tmp = data[1];
                    message_queue_write(&uart2driver_queue_id,tmp);
                    break;
                case 0xA1:
                    update_mcu_recv(data[6]);
                    break;
                default:
                    post_back.result = status_table[match_index(0x07)].value;
                    strcpy(post_back.debug,status_table[match_index(0x07)].str);
                    user_app_reply_func(&post_back);
                    break;
                }
            }else
            {
                post_back.result = status_table[match_index(0x06)].value;
                strcpy(post_back.debug,status_table[match_index(0x06)].str);
                user_app_reply_func(&post_back);
            }
        }
    }
}

void app_receive_thread(void *param)
{
    message_queue_init(&uart2driver_queue_id, sizeof(uint8_t), 15);
    // RequestMcuInfo(NULL); //若无需MCU升级 则开启
    while (1)
    {
        uint8_t CargoData[8] = {0};
        uint32_t lenth = 0;
        uint8_t i = 0;
        post_back_t *post_back = message_queue_read((msg_queue_thread *)param); 
        printf("[thread]app_receive_thread!cmd[%d]\n",post_back->cmd);
        if (1000 == post_back->cmd)
        {
            if (0 == strlen(sub_version[(post_back->num + 16 -1)/16 -1]))
            {
                post_back->result = false;
                strcpy(post_back->debug,"NO_MATCH");
                user_app_reply_func(post_back);
                continue;
            }
            for ( post_back->current = 1; post_back->current <= post_back->total; post_back->current++)
            {   
                lenth = CargoDigital(post_back->num + 16,CargoData);
                bool ret = uart_driver_deal(CargoData,lenth,post_back);
                user_app_reply_func(post_back);
                if(false == ret)
                {
                    break;
                }
                UnSleep_MS(100);
            }
        }else if (1002 == post_back->cmd)
        {
            if (0 == sub_count)
            {
                post_back->result = false;
                strcpy(post_back->debug,"NO_MATCH");
                user_app_reply_func(post_back);
                continue;
            }
            post_back->total = sub_count;
            for ( i = 0; i < 7; i++)
            {
                if (strlen(sub_version[i])>0)
                {
                    lenth = Replenishment(i+1,CargoData);
                    bool ret = uart_driver_deal(CargoData,lenth,post_back);
                    user_app_reply_func(post_back);
                    if(false == ret)
                    {
                        break;
                    }
                }
                UnSleep_MS(100);
            }
        }
        message_queue_message_free((msg_queue_thread *)param, post_back);
    }
}
//VGLVRA866193054533142 

#endif
