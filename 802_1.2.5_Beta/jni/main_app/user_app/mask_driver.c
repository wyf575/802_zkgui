/**********************************************************************************************
 * 此文件适配了两个厂家刀头，鼎基刀头和MQX_MK_2020_25刀头
 * 1、新厂家刀头，和老厂家鼎基刀头协议不完全一样，新厂家鼎基刀头仅恢复口罩卡住和恢复切刀位置用到了crc校验
 * 2、新厂家刀头，控制逻辑和老厂家刀头不同，
 * 3、新厂家刀头，站地址拨码开关同老厂家不同，老厂家全OFF时地址0，新厂家全OFF时地址0xFF。
 * 4、代码适配新厂家刀头时对刀头地址做了转换，新厂家刀头拨码开关不拨时地址为0xFF,转换后对服务器而言，就是地址0
***********************************************************************************************/

#include "mask_driver.h"
#if (USER_APP_TYPE==USER_APP_NEW2_MASK) //add by wyf 2022 07 27
#include "main_app/hardware_tool/user_uart.h"
#include "user_app.h"

/*框架相关全局静态变量*/
static msg_queue_thread uart2driver_queue_id = {0};

volatile bool NothingFlag = false;//有口罩标志
volatile bool busy_flag = false;//2021.10.18 ethan add

volatile bool idle_status = true;
static uint8_t reset_data[] = {0x7F, 0x05, 0x00, 0x01, 0x2F, 0x01, 0x1F, 0x03, 0x03, 0x0A, 0x2F};//退货指令
static uint8_t idle_data[] = {0x7F,0x05,0x00 ,0x01 ,0x2F ,0x01 ,0x1F ,0x09 ,0x03 ,0xC1,0xC0};//读设备传感器状态
static uint8_t out_data[] = {0x7F, 0x06, 0x00, 0x01, 0x2F, 0x01, 0x1F, 0x05, 0x0A, 0x03,0x92, 0xCA};//出票指令
//static uint8_t stuck_data[] = {0x1F, 0x0F, 0x00, 0x05, 0x02, 0x00,0xFF, 0xFF};//恢复口罩卡住错误状态
//static uint8_t cuterr_data[] = {0x1F, 0x0F, 0x00, 0x05, 0x01, 0x00, 0xFF, 0xFF};//恢复切刀初始位置

#define STATUS_TABLE_LEN 11
#define INFO_TABLE_TOTAL_LEN 6
#define INFO_TABLE_REPORT_LEN 4
#define MATCH_TABLE_LEN 9
#define TIMEOUT_ALARM_COUNT 60

/*
机头状态，常量 C++结构体初始化要求按顺序
*/
static status_table_t status_table[STATUS_TABLE_LEN] = {
    [OUT_ABNORMAL] = {"OUT_ABNORMAL", false},
    [OUT_NORMAL] = {"OUT_NORMAL", true},
    [MOTOR_TIMEOUT_ERROR] = {"MOTOR_TIMEOUT_ERROR", 1},
    [MOTOR_TIMEOUT_NORMAL] = {"MOTOR_TIMEOUT_NORMAL", 2},
    [DATA_FORMAT_ERROR] = {"DATA_FORMAT_ERROR", false},
    [HAVE_NOTHING] = {"HAVE_NOTHING", 1},
    [HAVE_THING] = {"HAVE_THING", 2},
    [TICKET_IN_OUTSIDE] = {"TICKET_IN_OUTSIDE",1},
    [TICKET_NOT_TAKEN] = {"TICKET_NOT_TAKEN",2},
    [STUCKING] = {"FAILED",1},
    [STATUS_NULL] = {"STATUS_NULL", 2},
};

/*
机头0状态 1有错误信息，2无错误信息
*/
static sq800_info_table_t sq800_info[LAST + 1][INFO_TABLE_TOTAL_LEN] = {
    {
        /*顺序不能改动 与match_table_t成员uindex挂钩*/
        [Nothing] = {"Nothing", {"STATUS_NULL", 2}},
        [TimeOut] = {"TimeOut", {"STATUS_NULL", 2}},
        [Ticket_In_Outside] = {"Ticket_In_Outside", {"STATUS_NULL", 2}},
        [Stucking] = {"Stucking", {"STATUS_NULL", 2}},
        /*下面的状态信息 不参与报警*/
        [out_status] = {"out_status", {"STATUS_NULL", 2}},
        [msg_debug] = {"msg_debug", {"STATUS_NULL", 2}},
    },
    {}
};



/*
刀头返回数据帧各字节数据说明，状态类型仅用到0x09-返回读传感器状态，0x05-返回出票命令
此数组元素名称与实际字节序不一致，byte4-7对应实际byte3-6。
状态1(传感器状态返回)，仅处理状态参数为5时的情况。
状态4(出口罩返回)，命令参数0xff，缺省。
状态5(错误恢复返回)，命令参数01恢复切刀错误，02恢复口罩卡住错误。
    {0x01, 0x05, 0x00, {CUT_LOCAL_NORMAL, HAVE_THING, STUCK_THING_NOMAL, MOTOR_TIMEOUT_NORMAL, STATUS_NULL, STATUS_NULL}},
    {0x01, 0x05, 0x01, {STATUS_NULL, HAVE_NOTHING, STATUS_NULL, MOTOR_TIMEOUT_NORMAL, STATUS_NULL, HAVE_NOTHING}},
    {0x01, 0x05, 0x02, {STATUS_NULL, HAVE_NOTHING, STATUS_NULL, MOTOR_TIMEOUT_NORMAL, STATUS_NULL, HAVE_NOTHING}},
    {0x01, 0x05, 0x03, {CUT_LOCAL_ERROR, STATUS_NULL, STATUS_NULL, MOTOR_TIMEOUT_NORMAL, STATUS_NULL, CUT_LOCAL_ERROR}},
    {0x01, 0x05, 0x04, {STATUS_NULL, STATUS_NULL, STUCK_EEROR, MOTOR_TIMEOUT_NORMAL, STATUS_NULL, STUCK_EEROR}},
    {0x04, 0xFF, 0x00, {STATUS_NULL, STATUS_NULL, STATUS_NULL, MOTOR_TIMEOUT_NORMAL, OUT_NORMAL, OUT_NORMAL}},
    {0x04, 0xFF, 0x01, {STATUS_NULL, HAVE_NOTHING, STATUS_NULL, MOTOR_TIMEOUT_NORMAL, OUT_ABNORMAL, HAVE_NOTHING}},
    {0x04, 0xFF, 0x02, {STATUS_NULL, STATUS_NULL, STATUS_NULL, MOTOR_TIMEOUT_NORMAL, OUT_ABNORMAL, BLACK_FLAG_ERROR}},
    {0x04, 0xFF, 0x03, {CUT_LOCAL_ERROR, STATUS_NULL, STATUS_NULL, MOTOR_TIMEOUT_NORMAL, OUT_ABNORMAL, CUT_LOCAL_ERROR}},
    {0x04, 0xFF, 0x04, {STATUS_NULL, STATUS_NULL, STUCK_EEROR, MOTOR_TIMEOUT_NORMAL, OUT_ABNORMAL, STUCK_EEROR}},
    {0x05, 0x01, 0x00, {CUT_LOCAL_NORMAL, STATUS_NULL, STATUS_NULL, MOTOR_TIMEOUT_NORMAL, STATUS_NULL, CUT_LOCAL_NORMAL}},
    {0x05, 0x02, 0x00, {STATUS_NULL, STATUS_NULL, STUCK_THING_NOMAL, MOTOR_TIMEOUT_NORMAL, STATUS_NULL, STUCK_THING_NOMAL}},
*/
match_table_t match_table[MATCH_TABLE_LEN] = {
    //byte4 byte5 byte7
    // byte8 byte9  对应 data[7] [8]
    {0x05, 0x00, OUT_NORMAL},
    {0x05, 0x01, STUCKING},
    {0x05, 0x02, TICKET_IN_OUTSIDE},
    {0x05, 0x03, TICKET_NOT_TAKEN},
    {0x05, 0x04, HAVE_NOTHING},
    {0x09, 0x00, HAVE_THING},
    {0x09, 0x01, STUCKING},
    {0x09, 0x02, TICKET_IN_OUTSIDE},
    {0x09, 0x04, HAVE_NOTHING},
};

/*
更新刀头状态
*/
bool update_device_status(uint8_t *data)
{
    uint8_t i = 0, j = 0;
    printf("*************************enter update_device_status ************************\n");
    for (j = 0; j < MATCH_TABLE_LEN; j++)
    {
        /*仅处理5 9两种状态，且状态和相应数据必须对应*/
        if (data[7] == match_table[j].byte8 && data[8] == match_table[j].byte9)
        {
            //printf("*************************update_device_status ************************data[6]=%d,data[7]=%d \n",data[6],data[7]);
            /*状态5：出口罩返回; 状态9初始化检测|| data[7] == 0x05  */
            if (data[7] == 0x09 || data[7] == match_table[j].byte8)
            {

                switch(match_table[j].uindex)
                {
                    case HAVE_THING:
                        printf("HAVE_THING\n");
                        sq800_info[NOW][Nothing].status = status_table[match_table[j].uindex];
                        sq800_info[NOW][Stucking].status = status_table[10];
                        sq800_info[NOW][TimeOut].status = status_table[10];
                        sq800_info[NOW][Ticket_In_Outside].status = status_table[10];
                        sq800_info[NOW][msg_debug].status = status_table[10];
                        //sq800_info[NOW][msg_debug].status = status_table[match_table[j].uindex];
                        //sq800_info[NOW][msg_debug].status = status_table[match_table[j].uindex];
                    break;
                    case HAVE_NOTHING:
                        printf("HAVE_NOTHING\n");
                        sq800_info[NOW][Nothing].status = status_table[match_table[j].uindex];
                        sq800_info[NOW][msg_debug].status = status_table[match_table[j].uindex];
                        sq800_info[NOW][out_status].status = status_table[OUT_ABNORMAL];
                    break;
                    case OUT_NORMAL:
                        printf("OUT_NORMAL\n");
                        sq800_info[NOW][out_status].status = status_table[match_table[j].uindex];
                        sq800_info[NOW][msg_debug].status = status_table[match_table[j].uindex];
                    break;
                    case STUCKING:
                        printf("TICKET_SEND_ERRO\n");
                        sq800_info[NOW][Stucking].status = status_table[match_table[j].uindex];
                        sq800_info[NOW][msg_debug].status = status_table[match_table[j].uindex];
                        sq800_info[NOW][out_status].status = status_table[OUT_ABNORMAL];
                    break;
                    case TICKET_IN_OUTSIDE:
                        printf("TICKET_IN_OUTSIDE 4\n");
                        sq800_info[NOW][Ticket_In_Outside].status = status_table[match_table[j].uindex];
                        sq800_info[NOW][msg_debug].status = status_table[match_table[j].uindex];
                        sq800_info[NOW][out_status].status = status_table[OUT_ABNORMAL];
                        
                    break;
                    case TICKET_NOT_TAKEN:
                        printf("TICKET_NOT_TAKEN 3\n");
                        sq800_info[NOW][msg_debug].status = status_table[match_table[j].uindex];
                        sq800_info[NOW][out_status].status = status_table[OUT_NORMAL];
                    break;
                    default:
                    break;
                }
                printf("update_device_status = %s\n",sq800_info[NOW][msg_debug].status.str);
                printf("######################*************************update_device_status true************************ %d\n",__LINE__);
                return true;
            }
        }
    }
    printf("*************************update_device_status false************************\n");
    return false;
}

uint16_t crc16_xmode_custom(uint8_t *data, uint32_t lenth)
{
    /* add by wyf
    uint16_t wCRCin = 0x0000;
    uint16_t wCPoly = 0x8005;
    uint8_t i = 0, j = 0;
    for (j = 0; j < lenth; j++)
    {
        for (i = 0; i < 8; i++)
        {
            uint8_t bit_value = ((data[j] >> (7 - i)) & 0x01);
            uint8_t c15_value = ((wCRCin >> 15) & 0x01);
            wCRCin = wCRCin << 1;
            if (bit_value != c15_value)
            {
                wCRCin = wCRCin ^ wCPoly;
            }
        }
    }
    wCRCin = wCRCin & 0xFFFF;
    return (wCRCin ^ 0x0000);
    */

    uint8_t i= 0;
    uint16_t uiCRCResult = 0x0000;
    
    while(lenth--)
    {
        uiCRCResult ^= (uint8_t)(*data++) << 8;
        for (i = 0; i < 8; ++i)
        {
            if (uiCRCResult & 0x8000)
                uiCRCResult = (uiCRCResult << 1) ^ 0x1021;
            else
                uiCRCResult <<= 1;
        }
    }
    uiCRCResult = (uiCRCResult>>8)| ((uiCRCResult&0xff)<<8);
    
    return uiCRCResult;

}

/*
校验串口接收到的数据
*/
static bool check_data_legality(uint8_t *data, uint32_t lenth)
{
    if (0x7F != data[0])
    {
        //printf("**************************** data check_data_legality fail****************************\n");
        return false;
    }
    else
    {
        uint16_t crc_result = crc16_xmode_custom(data, lenth - 2);
        if (data[lenth - 2] != (crc_result >> 8) || data[lenth - 1] != (crc_result & 0xFF))
        {
           // printf("****************************check_data_legality fail****************************\n");
            return false;
        }
    }
     //printf("****************************check_data_legality successfull****************************\n");
    return true;
}

/*
发送串口消息，等待机头恢复串口消息(有软件超时)，超时后标记超时错误。更新设备状态信息(仅不报警那块)
return:true机头正常返回数据，false机头串口通信超时
*/
static bool uart_driver_deal(uint8_t *data, uint32_t lenth)
{
    uint8_t i = 0;
    static uint32_t timeout_count = 0;
    UnSleep_MS(500); /* 非常重要!!! 为从机释放总线留足时间*/
    printf("uart_driver_deal start[%d]=======================[%ld]\n",i,time(NULL));
    user_uart_send(0, data, lenth);
    uint8_t *data_byte = message_queue_timeout(&uart2driver_queue_id,200);//等20s
    if (NULL != data_byte)//等待到机头响应
    {
        printf("uart_driver_deal end=======================[%ld]\n",time(NULL));
        timeout_count = 0;
        message_queue_message_free(&uart2driver_queue_id,data_byte);
        return true;
    }
    else
    {
        timeout_count++;
        printf("uart_driver_deal timeout[%d]=======================[%ld]\n",timeout_count,time(NULL));
        if (timeout_count>=TIMEOUT_ALARM_COUNT)
        {
            timeout_count = 0;
            sq800_info[NOW][TimeOut].status = status_table[MOTOR_TIMEOUT_ERROR];//ethan.xu 2021.9.7注释
            
        }
    }
    
    sq800_info[NOW][out_status].status = status_table[OUT_ABNORMAL];//ethan.xu 2021.9.7注释
    sq800_info[NOW][msg_debug].status = status_table[MOTOR_TIMEOUT_ERROR];//ethan.xu 2021.9.7注释
    printf("sq800_info[NOW][msg_debug].status ========================%s\n",sq800_info[NOW][msg_debug].status);
    
    return false;
}

void add_crc16_param(uint8_t *data,uint8_t len)
{
    uint16_t crc16_result = crc16_xmode_custom(data, len - 2);
    data[len-2] = crc16_result >> 8;
    data[len-1] = crc16_result & 0xFF;
}

void init_cmd_param(void)
{
    add_crc16_param(reset_data,sizeof(reset_data));
    add_crc16_param(idle_data,sizeof(idle_data));
    add_crc16_param(out_data,sizeof(out_data));
    //add_crc16_param(stuck_data,sizeof(stuck_data));
    //add_crc16_param(cuterr_data,sizeof(cuterr_data));
}

/*
检测机头状态
1、读取机头传感器状态
2、状态变化后更新状态至服务器
return true机头回了消息，false机头未回消息
*/
//bool check_status_report(void)//ethan.xu 2021.9.7注释
bool check_status_report(uint8_t add)//2021.10.18 ethan add
{
    uint8_t i = 0;
    //bool busy_flag = false;//2021.10.18删除
    //bool result;

    // while(busy_flag)//2021.10.18删除
    // {
    //     UnSleep_MS(200);
    //     printf("check_status_report busy[%ld]=====================\n",time(NULL));
    // }
    busy_flag = true;
    printf("check_status_report start[%ld]=====================\n",time(NULL));
    bool result = uart_driver_deal(idle_data, sizeof(idle_data));//阻塞读设备传感器状态//ethan.xu 2021.9.7注释


    /*ethan.xu 2021.9.7注释 */
    cJSON *array = cJSON_CreateArray();

    for (i = 0; i < INFO_TABLE_REPORT_LEN; i++)//报警状态
    {
        if (sq800_info[LAST][i].status.str != sq800_info[NOW][i].status.str)
        {
            sq800_info[LAST][i].status = sq800_info[NOW][i].status;
            //准备拼凑上报的信息
            cJSON *item = cJSON_CreateObject();
            create_json_table(item, J_String, "key", sq800_info[NOW][i].str);
            create_json_table(item, J_Int, "status", sq800_info[NOW][i].status.value.value_c);
            create_json_table(array, J_Array, item);
        }
    }
   

    if (cJSON_GetArraySize(array)>0)
    {
        user_report_alarm_func(array);//3816
    }else
    {
        cJSON_Delete(array);
    }
    printf("check_status_report end[%ld]=====================\n",time(NULL));
    busy_flag = false;
    return result;
}

/*
出货
1、出货前更新出货状态为正常出货，出货后再次更新出货状态
*/
void single_thing_out(uint8_t add)
{
    printf("single_thing_out start========================[%ld]\n",time(NULL));
    uint8_t try_count = 3;
    bool ret;

    sq800_info[NOW][out_status].status = status_table[OUT_NORMAL];

    
    ret = uart_driver_deal(out_data,sizeof(out_data));//与机头通信正常返回true,通信超时返回false//ethan.xu 2021.9.7注释
    if(ret)
    {
        /*ethan.xu 2021.9.7注释*/
        //出货异常时，尝试使用指令 恢复异常状态，最多尝试3次，而后再次出货
        while (sq800_info[NOW][out_status].status.str == status_table[OUT_ABNORMAL].str && try_count)
        {
            
            while (try_count)
            {
                try_count -- ;
                /*
                if (sq800_info[NOW][Stucking].status.str == status_table[STUCK_EEROR].str)//卡口罩
                {
                    //uart_driver_deal(stuck_data,sizeof(stuck_data));
                    if (sq800_info[NOW][Stucking].status.str == status_table[STUCK_THING_NOMAL].str)//卡口罩恢复正常
                    {
                        break;
                    }
                }
                if (sq800_info[NOW][CutError].status.str == status_table[CUT_LOCAL_ERROR].str)//切刀位置错误
                {
                    //uart_driver_deal(cuterr_data,sizeof(cuterr_data));
                    if (sq800_info[NOW][CutError].status.str == status_table[CUT_LOCAL_NORMAL].str)//切刀位置错误恢复正常
                    {
                        break;
                    }
                }
                */
               /*
                ret = uart_driver_deal(out_data,sizeof(out_data));
                UnSleep_S(15);
                if(ret) break;
                add by wyf 出货失败三次机会*/
            }
        }
 

    }
    printf("single_thing_out end========================[%ld]\n",time(NULL));
}

void HexToStr(uint8_t *hexSrc,char *destStr, int hexLen)
{
    char ddl, ddh;
    int i;
    for (i = 0; i < hexLen; i++)
    {
        ddh = 48 + hexSrc[i] / 16;
        ddl = 48 + hexSrc[i] % 16;
        if (ddh > 57)
            ddh = ddh + 7;
        if (ddl > 57)
            ddl = ddl + 7;
        destStr[i * 2] = ddh;
        destStr[i * 2 + 1] = ddl;
    }
    destStr[hexLen * 2] = '\0';
}

/*
接收串口数据
*/
void uart_receive_thread(void *param)
{
    uint8_t *data = (uint8_t *)malloc(1024);
    while (1)
    {
        printf("5min[%s]5min\n",__FUNCTION__);
        printf("[thread]uart_receive_thread!\n");
        memset(data, 0, 1024);
        int i = 0;
        int lenth = user_uart_receive(data,1023);
        for (i = 0; i < lenth; i++)
        {
            printf("r[%d][%02X] ", i,data[i]);
            
        }
        //printf("#################################uart_receive_thread#################################\n");
        if (uart2driver_queue_id.max_depth > 0)
        {
            if (true == check_data_legality(data, lenth))//crc校验
            {
                /*解析串口数据 更新设备状态*/
                if(true == update_device_status(data))
                {
                    uint8_t *tmp = message_queue_message_alloc_blocking(&uart2driver_queue_id);
                    *tmp = data[0];
                    message_queue_write(&uart2driver_queue_id,tmp);
                }
                else
                {
                    post_back_t post_back = {0};
                    post_back.result = false;
                    char tmpStr[50] = {0};
                    //HexToStr(data,tmpStr,lenth);
                    snprintf(post_back.debug,50,"UNKNOWN_REPLY[%s]",tmpStr);
                    user_app_reply_func(&post_back);
                }
            }
            else
            {
                post_back_t post_back = {0};
                post_back.result = status_table[DATA_FORMAT_ERROR].value.value_b;
                strcpy(post_back.debug, status_table[DATA_FORMAT_ERROR].str);
                user_app_reply_func(&post_back);
            }
        }
    }
}

/*
控制出货线程，入口参数为：app2driver_queue_id
一次出货支持连续出多个口罩;
1、首先检测机头状态；2、再进行出货
*/
void app_receive_thread(void *param)
{
    message_queue_init(&uart2driver_queue_id, sizeof(uint8_t), 15);
    init_cmd_param();
    while (1)
    {
        uint8_t add = 1;
        printf("5min[%s]5min\n",__FUNCTION__);
        printf("[thread]app_receive_thread!\n");
        post_back_t *post_back = message_queue_read((msg_queue_thread *)param);

        idle_status = false;
        UnSleep_MS(200);//等待机头状态检测线程，检测完成
        
        for (post_back->current = 1; post_back->current <= post_back->total; post_back->current++)
        {
           
            printf("app_receive_thread call check_status_report\n");
            while(busy_flag)//2021.10.18 ethan add
            {
                UnSleep_MS(200);
                printf("check_status_report busy[%ld]=====================\n",time(NULL));
            }
            bool result = check_status_report(add);//返回机头有无回消息
            //printf("*****************result = %d**********************\n",result);
            /*ethan.xu 2021.9.7注释*/
            if (sq800_info[NOW][Nothing].status.str == status_table[HAVE_NOTHING].str || 
                sq800_info[NOW][Stucking].status.str == status_table[STUCKING].str )
                 
            {
                sq800_info[NOW][out_status].status = status_table[OUT_ABNORMAL];
                //printf("sq800_info[NOW][Nothing].status.str = %s\n",sq800_info[NOW][Nothing].status.str);
                printf("###########OUT_ABNORMAL##########\n");
            }
            else if(true == result)
            {
                single_thing_out(1);//出货
            }

            post_back->result = sq800_info[NOW][out_status].status.value.value_b;
            strcpy(post_back->debug, sq800_info[NOW][msg_debug].status.str);
            //printf("app_receive_thread##########################%s\n",sq800_info[NOW][msg_debug].status.str);
            user_app_reply_func(post_back);//回复服务器

            if (sq800_info[NOW][out_status].status.str == status_table[OUT_ABNORMAL].str)
            {
                break;//终止出货
            }
            
          
        }
        idle_status = true;
        message_queue_message_free((msg_queue_thread *)param, post_back);
    }
}

/*
10s检测一次机头状态
如果当前机头状态为MOTOR_TIMEOUT_ERROR， 默认超时机头不回复 5s检测
*/
void idle_request_thread(void *param)
{
    uint8_t check_add = 0;

    while (NET_STATUS_OK!=get_app_network_status())
    {
        UnSleep_MS(100);
    }
    while (1)
    {
        printf("5min[%s]5min\n",__FUNCTION__);
        if (true == idle_status)//出货时不检查状态 默认超时机头不回复 5s检测
        {
            if(sq800_info[NOW][msg_debug].status.str == status_table[MOTOR_TIMEOUT_ERROR].str)
            {
                check_status_report(1);
                UnSleep_S(5);
                printf("5s######################idle_request_thread call check_status_report ******************MOTOR_TIMEOUT_ERROR\n");
                continue;      
            }else
            {
                check_status_report(1);
                printf("10s######################idle_request_thread call check_status_report ******************\n");
            }
            
            printf("idle_request_thread call check_status_report\n");         
        }
        UnSleep_S(10);
    }
}
#elif (USER_APP_TYPE==USER_APP_MASK)
#include "main_app/hardware_tool/user_uart.h"
#include "user_app.h"
#define MASK_DRIVER_NUM 4
volatile bool busy_flag = false;//2021.10.18 ethan add

/*框架相关全局静态变量*/
static msg_queue_thread uart2driver_queue_id = {0};

/*业务相关全局静态变量*/
volatile bool idle_status = true;
static uint8_t idle_data[] = {0x1F, 0x0F, 0x00, 0x01, 0x05, 0x00, 0xFF, 0xFF};//读设备传感器状态
static uint8_t out_data[] = {0x1F, 0x0F, 0x00, 0x04, 0x02, 0x04, 0xC8, 0x00, 0x00, 0x00,0xFF, 0xFF};//出票命令
static uint8_t stuck_data[] = {0x1F, 0x0F, 0x00, 0x05, 0x02, 0x00, 0xFF, 0xFF};//恢复口罩卡住错误状态
static uint8_t cuterr_data[] = {0x1F, 0x0F, 0x00, 0x05, 0x01, 0x00, 0xFF, 0xFF};//恢复切刀初始位置

/*ethan.xu 2021.9.7添加，新增一个机头控制,start*/
static uint8_t idle_data_one[] = {0x1F, 0x0F, 0x01, 0x01, 0x05, 0x00, 0xFF, 0xFF};//读设备传感器状态
static uint8_t out_data_one[] = {0x1F, 0x0F, 0x01, 0x04, 0x02, 0x04, 0xC8, 0x00, 0x00, 0x00,0xFF, 0xFF};//出票命令
static uint8_t stuck_data_one[] = {0x1F, 0x0F, 0x01, 0x05, 0x02, 0x00, 0xFF, 0xFF};//恢复口罩卡住错误状态
static uint8_t cuterr_data_one[] = {0x1F, 0x0F, 0x01, 0x05, 0x01, 0x00, 0xFF, 0xFF};//恢复切刀初始位置

static uint8_t idle_data_two[] = {0x1F, 0x0F, 0x02, 0x01, 0x05, 0x00, 0xFF, 0xFF};//读设备传感器状态
static uint8_t out_data_two[] = {0x1F, 0x0F, 0x02, 0x04, 0x02, 0x04, 0xC8, 0x00, 0x00, 0x00,0xFF, 0xFF};//出票命令
static uint8_t stuck_data_two[] = {0x1F, 0x0F, 0x02, 0x05, 0x02, 0x00, 0xFF, 0xFF};//恢复口罩卡住错误状态
static uint8_t cuterr_data_two[] = {0x1F, 0x0F, 0x02, 0x05, 0x01, 0x00, 0xFF, 0xFF};//恢复切刀初始位置

static uint8_t idle_data_three[] = {0x1F, 0x0F, 0x03, 0x01, 0x05, 0x00, 0xFF, 0xFF};//读设备传感器状态
static uint8_t out_data_three[] = {0x1F, 0x0F, 0x03, 0x04, 0x02, 0x04, 0xC8, 0x00, 0x00, 0x00,0xFF, 0xFF};//出票命令
static uint8_t stuck_data_three[] = {0x1F, 0x0F, 0x03, 0x05, 0x02, 0x00, 0xFF, 0xFF};//恢复口罩卡住错误状态
static uint8_t cuterr_data_three[] = {0x1F, 0x0F, 0x03, 0x05, 0x01, 0x00, 0xFF, 0xFF};//恢复切刀初始位置
/*ethan.xu 2021.9.7添加，新增一个机头控制,end*/

/*业务相关宏定义 与数据表相关*/
#define STATUS_TABLE_LEN 13
#define INFO_TABLE_TOTAL_LEN 6
#define INFO_TABLE_REPORT_LEN 4
#define MATCH_TABLE_LEN 13
#define TIMEOUT_ALARM_COUNT 120

/*
机头状态，常量
*/
static status_table_t status_table[STATUS_TABLE_LEN] = {
    [OUT_ABNORMAL] = {"OUT_ABNORMAL", false},
    [OUT_NORMAL] = {"OUT_NORMAL", true},
    [MOTOR_TIMEOUT_ERROR] = {"MOTOR_TIMEOUT_ERROR", 1},
    [MOTOR_TIMEOUT_NORMAL] = {"MOTOR_TIMEOUT_NORMAL", 2},
    [DATA_FORMAT_ERROR] = {"DATA_FORMAT_ERROR", false},
    [HAVE_NOTHING] = {"HAVE_NOTHING", 1},
    [HAVE_THING] = {"HAVE_THING", 2},
    [CUT_LOCAL_ERROR] = {"CUT_LOCAL_ERROR", 1},
    [CUT_LOCAL_NORMAL] = {"CUT_LOCAL_NORMAL", 2},
    [STUCK_EEROR] = {"STUCK_EEROR", 1},
    [STUCK_THING_NOMAL] = {"STUCK_THING_NOMAL", 2},
    [BLACK_FLAG_ERROR] = {"BLACK_FLAG_ERROR", 1},
    [STATUS_NULL] = {"STATUS_NULL", 2},
};

/*
机头0状态 1有错误信息，2无错误信息
*/
static sq800_info_table_t sq800_info[LAST + 1][INFO_TABLE_TOTAL_LEN] = {
    {
        /*顺序不能改动 与match_table_t成员uindex挂钩*/
        [CutError] = {"CutError", {"STATUS_NULL", 2}},
        [Nothing] = {"Nothing", {"STATUS_NULL", 2}},
        [Stucking] = {"Stucking", {"STATUS_NULL", 2}},
        [TimeOut] = {"TimeOut", {"STATUS_NULL", 2}},

        /*下面的状态信息 不参与报警*/
        [out_status] = {"out_status", {"STATUS_NULL", 2}},
        [msg_debug] = {"msg_debug", {"STATUS_NULL", 2}},
    },
    {}
};

/*ethan.xu 2021.9.7添加，新增一个机头控制,start*/
static sq800_info_table_t sq800_info_one[LAST + 1][INFO_TABLE_TOTAL_LEN] = {
    {
        /*顺序不能改动 与match_table_t成员uindex挂钩*/
        [CutError] = {"CutError", {"STATUS_NULL", 2}},
        [Nothing] = {"Nothing", {"STATUS_NULL", 2}},
        [Stucking] = {"Stucking", {"STATUS_NULL", 2}},
        [TimeOut] = {"TimeOut", {"STATUS_NULL", 2}},

        /*下面的状态信息 不参与报警*/
        [out_status] = {"out_status", {"STATUS_NULL", 2}},
        [msg_debug] = {"msg_debug", {"STATUS_NULL", 2}},
    },
    {}
};
static sq800_info_table_t sq800_info_two[LAST + 1][INFO_TABLE_TOTAL_LEN] = {
    {
        /*顺序不能改动 与match_table_t成员uindex挂钩*/
        [CutError] = {"CutError", {"STATUS_NULL", 2}},
        [Nothing] = {"Nothing", {"STATUS_NULL", 2}},
        [Stucking] = {"Stucking", {"STATUS_NULL", 2}},
        [TimeOut] = {"TimeOut", {"STATUS_NULL", 2}},

        /*下面的状态信息 不参与报警*/
        [out_status] = {"out_status", {"STATUS_NULL", 2}},
        [msg_debug] = {"msg_debug", {"STATUS_NULL", 2}},
    },
    {}
};
static sq800_info_table_t sq800_info_three[LAST + 1][INFO_TABLE_TOTAL_LEN] = {
    {
        /*顺序不能改动 与match_table_t成员uindex挂钩*/
        [CutError] = {"CutError", {"STATUS_NULL", 2}},
        [Nothing] = {"Nothing", {"STATUS_NULL", 2}},
        [Stucking] = {"Stucking", {"STATUS_NULL", 2}},
        [TimeOut] = {"TimeOut", {"STATUS_NULL", 2}},

        /*下面的状态信息 不参与报警*/
        [out_status] = {"out_status", {"STATUS_NULL", 2}},
        [msg_debug] = {"msg_debug", {"STATUS_NULL", 2}},
    },
    {}
};
/*ethan.xu 2021.9.7添加，新增一个机头控制,end*/

/*
刀头返回数据帧各字节数据说明，状态类型仅用到0x01-返回读传感器状态，0x04-返回出票命令，0x05-返回错误恢复。
此数组元素名称与实际字节序不一致，byte4-7对应实际byte3-6。
状态1(传感器状态返回)，仅处理状态参数为5时的情况。
状态4(出口罩返回)，命令参数0xff，缺省。
状态5(错误恢复返回)，命令参数01恢复切刀错误，02恢复口罩卡住错误。
*/
match_table_t match_table[MATCH_TABLE_LEN] = {
    //byte4 byte5 byte7
    {0x01, 0x05, 0x00, {CUT_LOCAL_NORMAL, HAVE_THING, STUCK_THING_NOMAL, MOTOR_TIMEOUT_NORMAL, STATUS_NULL, STATUS_NULL}},
    {0x01, 0x05, 0x01, {STATUS_NULL, HAVE_NOTHING, STATUS_NULL, MOTOR_TIMEOUT_NORMAL, STATUS_NULL, HAVE_NOTHING}},
    {0x01, 0x05, 0x02, {STATUS_NULL, HAVE_NOTHING, STATUS_NULL, MOTOR_TIMEOUT_NORMAL, STATUS_NULL, HAVE_NOTHING}},
    {0x01, 0x05, 0x03, {CUT_LOCAL_ERROR, STATUS_NULL, STATUS_NULL, MOTOR_TIMEOUT_NORMAL, STATUS_NULL, CUT_LOCAL_ERROR}},
    {0x01, 0x05, 0x04, {STATUS_NULL, STATUS_NULL, STUCK_EEROR, MOTOR_TIMEOUT_NORMAL, STATUS_NULL, STUCK_EEROR}},
    {0x04, 0xFF, 0x00, {STATUS_NULL, STATUS_NULL, STATUS_NULL, MOTOR_TIMEOUT_NORMAL, OUT_NORMAL, OUT_NORMAL}},
    {0x04, 0xFF, 0x01, {STATUS_NULL, HAVE_NOTHING, STATUS_NULL, MOTOR_TIMEOUT_NORMAL, OUT_ABNORMAL, HAVE_NOTHING}},
    {0x04, 0xFF, 0x02, {STATUS_NULL, STATUS_NULL, STATUS_NULL, MOTOR_TIMEOUT_NORMAL, OUT_ABNORMAL, BLACK_FLAG_ERROR}},
    {0x04, 0xFF, 0x03, {CUT_LOCAL_ERROR, STATUS_NULL, STATUS_NULL, MOTOR_TIMEOUT_NORMAL, OUT_ABNORMAL, CUT_LOCAL_ERROR}},
    {0x04, 0xFF, 0x04, {STATUS_NULL, STATUS_NULL, STUCK_EEROR, MOTOR_TIMEOUT_NORMAL, OUT_ABNORMAL, STUCK_EEROR}},
    {0x05, 0x01, 0x00, {CUT_LOCAL_NORMAL, STATUS_NULL, STATUS_NULL, MOTOR_TIMEOUT_NORMAL, STATUS_NULL, CUT_LOCAL_NORMAL}},
    {0x05, 0x02, 0x00, {STATUS_NULL, STATUS_NULL, STUCK_THING_NOMAL, MOTOR_TIMEOUT_NORMAL, STATUS_NULL, STUCK_THING_NOMAL}},
};

/*
更新刀头状态
*/
bool update_device_status(uint8_t *data)
{
    uint8_t i = 0, j = 0;
    /*ethan.xu 2021.9.7添加，新增一个机头控制,start*/
    uint8_t add;
    add = data[2];
    /*ethan.xu 2021.9.7添加，新增一个机头控制,end*/

    for (j = 0; j < MATCH_TABLE_LEN; j++)
    {
        /*仅处理1 4 5三种状态，且状态和相应数据必须对应*/
        if (data[3] == match_table[j].byte4 && data[6] == match_table[j].byte7)
        {
            /*状态4：出口罩返回; 状态5中切刀错误、切刀卡住错误;状态1中状态参数5(包括好多错误)*/
            if (data[3] == 0x04 || data[4] == match_table[j].byte5)
            {
                for (i = 0; i < INFO_TABLE_TOTAL_LEN; i++)
                {
                    //printf("match_table[%d].uindex[%d]==[%d]\n", j,i,match_table[j].uindex[i]);
                    if (match_table[j].uindex[i] != STATUS_NULL)
                    {
                        /*ethan.xu 2021.9.7添加，新增一个机头控制,start*/
                        if(0x00 == add)
                        {
                            sq800_info[NOW][i].status = status_table[match_table[j].uindex[i]];
                        }
                        else if(0x01 == add)
                        {
                            sq800_info_one[NOW][i].status = status_table[match_table[j].uindex[i]];
                        }
                        else if(0x02 == add)
                        {
                            sq800_info_two[NOW][i].status = status_table[match_table[j].uindex[i]];
                        }
                        else if(0x03 == add)
                        {
                            sq800_info_three[NOW][i].status = status_table[match_table[j].uindex[i]];
                        }
                        /*ethan.xu 2021.9.7添加，新增一个机头控制,end*/

                        //sq800_info[NOW][i].status = status_table[match_table[j].uindex[i]];//ethan.xu 2021.9.7注释
                        // printf("[%s][%s][%d]\n",sq800_info[NOW][i].str,sq800_info[NOW][i].status.str,sq800_info[NOW][i].status.value.value_c);
                    }
                }
                return true;
            }
        }
    }
    return false;
}

uint16_t crc16_xmode_custom(uint8_t *data, uint32_t lenth)
{
    uint16_t wCRCin = 0x0000;
    uint16_t wCPoly = 0x8005;
    uint8_t i = 0, j = 0;
    for (j = 0; j < lenth; j++)
    {
        for (i = 0; i < 8; i++)
        {
            uint8_t bit_value = ((data[j] >> (7 - i)) & 0x01);
            uint8_t c15_value = ((wCRCin >> 15) & 0x01);
            wCRCin = wCRCin << 1;
            if (bit_value != c15_value)
            {
                wCRCin = wCRCin ^ wCPoly;
            }
        }
    }
    wCRCin = wCRCin & 0xFFFF;
    return (wCRCin ^ 0x0000);
}

/*
校验串口接收到的数据
*/
static bool check_data_legality(uint8_t *data, uint32_t lenth)
{
    if (0x1F != data[0] || 0x0F != data[1])
    {
        return false;
    }
    else
    {
        uint16_t crc_result = crc16_xmode_custom(data, lenth - 2);
        if (data[lenth - 2] != (crc_result >> 8) || data[lenth - 1] != (crc_result & 0xFF))
        {
            return false;
        }
    }
    return true;
}

/*
发送串口消息，等待机头恢复串口消息(有软件超时)，超时后标记超时错误。更新设备状态信息(仅不报警那块)
return:true机头正常返回数据，false机头串口通信超时
*/
static bool uart_driver_deal(uint8_t *data, uint32_t lenth)
{
    uint8_t i = 0;
    static uint32_t timeout_count = 0;

    /*ethan.xu 2021.9.7添加，新增一个机头控制,start*/
    uint8_t add;
    add = data[2];
    /*ethan.xu 2021.9.7添加，新增一个机头控制,end*/

    for (i = 0; i < 2; i++)
    {
        // UnSleep_MS(500); /* 非常重要!!! 为从机释放总线留足时间*/
        printf("uart_driver_deal start[%d]=======================[%ld]\n",i,time(NULL));
        user_uart_send(0, data, lenth);
        uint8_t *data_byte = message_queue_timeout(&uart2driver_queue_id,50);//等5s
        if (NULL != data_byte)//等待到机头响应
        {
            printf("uart_driver_deal end=======================[%ld]\n",time(NULL));
            timeout_count = 0;
            message_queue_message_free(&uart2driver_queue_id,data_byte);
            return true;
        }
        else
        {
            timeout_count++;
            printf("uart_driver_deal timeout[%d]=======================[%ld]\n",timeout_count,time(NULL));
            if (timeout_count>=TIMEOUT_ALARM_COUNT)
            {
                timeout_count = 0;
                //sq800_info[NOW][TimeOut].status = status_table[MOTOR_TIMEOUT_ERROR];//ethan.xu 2021.9.7注释
                /*ethan.xu 2021.9.7添加，新增一个机头控制,start*/
                if(0x00 == add)
                {
                    sq800_info[NOW][TimeOut].status = status_table[MOTOR_TIMEOUT_ERROR];
                }
                else if(0x01 == add)
                {
                    sq800_info_one[NOW][TimeOut].status = status_table[MOTOR_TIMEOUT_ERROR];
                }
                else if(0x02 == add)
                {
                    sq800_info_two[NOW][TimeOut].status = status_table[MOTOR_TIMEOUT_ERROR];
                }
                else if(0x03 == add)
                {
                    sq800_info_three[NOW][TimeOut].status = status_table[MOTOR_TIMEOUT_ERROR];
                }
                /*ethan.xu 2021.9.7添加，新增一个机头控制,end*/
            }
        }
    }
    //sq800_info[NOW][out_status].status = status_table[OUT_ABNORMAL];//ethan.xu 2021.9.7注释
    //sq800_info[NOW][msg_debug].status = status_table[MOTOR_TIMEOUT_ERROR];//ethan.xu 2021.9.7注释

    /*ethan.xu 2021.9.7添加，新增一个机头控制,start*/
    if(0x00 == add)
    {
        sq800_info[NOW][out_status].status = status_table[OUT_ABNORMAL];
        sq800_info[NOW][msg_debug].status = status_table[MOTOR_TIMEOUT_ERROR];
    }
    else if(0x01 == add)
    {
        sq800_info_one[NOW][out_status].status = status_table[OUT_ABNORMAL];
        sq800_info_one[NOW][msg_debug].status = status_table[MOTOR_TIMEOUT_ERROR];
    }
    else if(0x02 == add)
    {
        sq800_info_two[NOW][out_status].status = status_table[OUT_ABNORMAL];
        sq800_info_two[NOW][msg_debug].status = status_table[MOTOR_TIMEOUT_ERROR];
    }
    else if(0x03 == add)
    {
        sq800_info_three[NOW][out_status].status = status_table[OUT_ABNORMAL];
        sq800_info_three[NOW][msg_debug].status = status_table[MOTOR_TIMEOUT_ERROR];
    }
    /*ethan.xu 2021.9.7添加，新增一个机头控制,end*/

    return false;
}

void add_crc16_param(uint8_t *data,uint8_t len)
{
    uint16_t crc16_result = crc16_xmode_custom(data, len - 2);
    data[len-2] = crc16_result >> 8;
    data[len-1] = crc16_result & 0xFF;
}

void init_cmd_param(void)
{
    add_crc16_param(idle_data,sizeof(idle_data));
    add_crc16_param(out_data,sizeof(out_data));
    add_crc16_param(stuck_data,sizeof(stuck_data));
    add_crc16_param(cuterr_data,sizeof(cuterr_data));

    /*ethan.xu 2021.9.7添加，新增一个机头控制,start*/
    add_crc16_param(idle_data_one,sizeof(idle_data_one));
    add_crc16_param(out_data_one,sizeof(out_data_one));
    add_crc16_param(stuck_data_one,sizeof(stuck_data_one));
    add_crc16_param(cuterr_data_one,sizeof(cuterr_data_one));

    add_crc16_param(idle_data_two,sizeof(idle_data_two));
    add_crc16_param(out_data_two,sizeof(out_data_two));
    add_crc16_param(stuck_data_two,sizeof(stuck_data_two));
    add_crc16_param(cuterr_data_two,sizeof(cuterr_data_two));

    add_crc16_param(idle_data_three,sizeof(idle_data_three));
    add_crc16_param(out_data_three,sizeof(out_data_three));
    add_crc16_param(stuck_data_three,sizeof(stuck_data_three));
    add_crc16_param(cuterr_data_three,sizeof(cuterr_data_three));
    /*ethan.xu 2021.9.7添加，新增一个机头控制,end*/
}

/*
检测机头状态
1、读取机头传感器状态
2、状态变化后更新状态至服务器
return true机头回了消息，false机头未回消息
*/
//bool check_status_report(void)//ethan.xu 2021.9.7注释
bool check_status_report(uint8_t add)//2021.10.18 ethan add
{
    uint8_t i = 0;
    //bool busy_flag = false;//2021.10.18删除
    bool result;

    // while(busy_flag)//2021.10.18删除
    // {
    //     UnSleep_MS(200);
    //     printf("check_status_report busy[%ld]=====================\n",time(NULL));
    // }
    busy_flag = true;
    printf("check_status_report start[%ld]=====================\n",time(NULL));
    //bool result = uart_driver_deal(idle_data, sizeof(idle_data));//阻塞读设备传感器状态//ethan.xu 2021.9.7注释

    /*ethan.xu 2021.9.7添加，新增一个机头控制,start*/
    cJSON *array = cJSON_CreateArray();

    if(0x00 == add)
    {
        result = uart_driver_deal(idle_data, sizeof(idle_data));//阻塞读设备传感器状态

        for (i = 0; i < INFO_TABLE_REPORT_LEN; i++)//报警状态
        {
            if (sq800_info[LAST][i].status.str != sq800_info[NOW][i].status.str)
            {
                sq800_info[LAST][i].status = sq800_info[NOW][i].status;
                //准备拼凑上报的信息
                cJSON *item = cJSON_CreateObject();
                create_json_table(item, J_Int, "digital", 1);
                create_json_table(item, J_String, "key", sq800_info[NOW][i].str);
                create_json_table(item, J_Int, "status", sq800_info[NOW][i].status.value.value_c);
                create_json_table(array, J_Array, item);
            }
        }
    }
    else if(0x01 == add)
    {
        result = uart_driver_deal(idle_data_one, sizeof(idle_data_one));//阻塞读设备传感器状态

        for (i = 0; i < INFO_TABLE_REPORT_LEN; i++)//报警状态
        {
            if (sq800_info_one[LAST][i].status.str != sq800_info_one[NOW][i].status.str)
            {
                sq800_info_one[LAST][i].status = sq800_info_one[NOW][i].status;
                /*准备拼凑上报的信息*/
                cJSON *item = cJSON_CreateObject();
                create_json_table(item, J_Int, "digital", 2);
                create_json_table(item, J_String, "key", sq800_info_one[NOW][i].str);
                create_json_table(item, J_Int, "status", sq800_info_one[NOW][i].status.value.value_c);
                create_json_table(array, J_Array, item);
            }
        }
    }
    else if(0x02 == add)
    {
        result = uart_driver_deal(idle_data_two, sizeof(idle_data_two));//阻塞读设备传感器状态

        for (i = 0; i < INFO_TABLE_REPORT_LEN; i++)//报警状态
        {
            if (sq800_info_two[LAST][i].status.str != sq800_info_two[NOW][i].status.str)
            {
                sq800_info_two[LAST][i].status = sq800_info_two[NOW][i].status;
                /*准备拼凑上报的信息*/
                cJSON *item = cJSON_CreateObject();
                create_json_table(item, J_Int, "digital", 3);
                create_json_table(item, J_String, "key", sq800_info_two[NOW][i].str);
                create_json_table(item, J_Int, "status", sq800_info_two[NOW][i].status.value.value_c);
                create_json_table(array, J_Array, item);
            }
        }
    }
    else if(0x03 == add)
    {
        result = uart_driver_deal(idle_data_three, sizeof(idle_data_three));//阻塞读设备传感器状态

        for (i = 0; i < INFO_TABLE_REPORT_LEN; i++)//报警状态
        {
            if (sq800_info_three[LAST][i].status.str != sq800_info_three[NOW][i].status.str)
            {
                sq800_info_three[LAST][i].status = sq800_info_three[NOW][i].status;
                /*准备拼凑上报的信息*/
                cJSON *item = cJSON_CreateObject();
                create_json_table(item, J_Int, "digital", 4);
                create_json_table(item, J_String, "key", sq800_info_three[NOW][i].str);
                create_json_table(item, J_Int, "status", sq800_info_three[NOW][i].status.value.value_c);
                create_json_table(array, J_Array, item);
            }
        }
    }
    /*ethan.xu 2021.9.7添加，新增一个机头控制,end*/

    /*ethan.xu 2021.9.7注释
    cJSON *array = cJSON_CreateArray();

    for (i = 0; i < INFO_TABLE_REPORT_LEN; i++)//报警状态
    {
        if (sq800_info[LAST][i].status.str != sq800_info[NOW][i].status.str)
        {
            sq800_info[LAST][i].status = sq800_info[NOW][i].status;
            //准备拼凑上报的信息
            cJSON *item = cJSON_CreateObject();
            create_json_table(item, J_String, "key", sq800_info[NOW][i].str);
            create_json_table(item, J_Int, "status", sq800_info[NOW][i].status.value.value_c);
            create_json_table(array, J_Array, item);
        }
    }
    */

    if (cJSON_GetArraySize(array)>0)
    {
        user_report_alarm_func(array);//3816
    }else
    {
        cJSON_Delete(array);
    }
    printf("check_status_report end[%ld]=====================\n",time(NULL));
    busy_flag = false;
    return result;
}

/*
出货
1、出货前更新出货状态为正常出货，出货后再次更新出货状态
*/
void single_thing_out(uint8_t add)
{
    printf("single_thing_out start========================[%ld]\n",time(NULL));
    uint8_t try_count = 3;
    bool ret;

    sq800_info[NOW][out_status].status = status_table[OUT_NORMAL];
    /*ethan.xu 2021.9.7添加，新增一个机头控制,start*/
    sq800_info_one[NOW][out_status].status = status_table[OUT_NORMAL];
    sq800_info_two[NOW][out_status].status = status_table[OUT_NORMAL];
    sq800_info_three[NOW][out_status].status = status_table[OUT_NORMAL];
    /*ethan.xu 2021.9.7添加，新增一个机头控制,end*/
    if(0 == add)
    {
        ret = uart_driver_deal(out_data,sizeof(out_data));
    }
    else if(1 == add)
    {
        ret = uart_driver_deal(out_data_one,sizeof(out_data_one));
    }
    else if(2 == add)
    {
        ret = uart_driver_deal(out_data_two,sizeof(out_data_two));
    }
    else if(3 == add)
    {
        ret = uart_driver_deal(out_data_three,sizeof(out_data_three));
    }
    /*ethan.xu 2021.9.7添加，新增一个机头控制,end*/
    
    //if(uart_driver_deal(out_data,sizeof(out_data)))//与机头通信正常返回true,通信超时返回false//ethan.xu 2021.9.7注释
    if(ret)
    {
        /*ethan.xu 2021.9.7注释
        //出货异常时，尝试使用指令 恢复异常状态，最多尝试3次，而后再次出货
        while (sq800_info[NOW][out_status].status.str == status_table[OUT_ABNORMAL].str && try_count)
        {
            while (try_count)
            {
                try_count -- ;
                if (sq800_info[NOW][Stucking].status.str == status_table[STUCK_EEROR].str)//卡口罩
                {
                    uart_driver_deal(stuck_data,sizeof(stuck_data));
                    if (sq800_info[NOW][Stucking].status.str == status_table[STUCK_THING_NOMAL].str)//卡口罩恢复正常
                    {
                        break;
                    }
                }
                if (sq800_info[NOW][CutError].status.str == status_table[CUT_LOCAL_ERROR].str)//切刀位置错误
                {
                    uart_driver_deal(cuterr_data,sizeof(cuterr_data));
                    if (sq800_info[NOW][CutError].status.str == status_table[CUT_LOCAL_NORMAL].str)//切刀位置错误恢复正常
                    {
                        break;
                    }
                }
            }
            if (0 != try_count)
            {
                uart_driver_deal(out_data,sizeof(out_data));
            }
        }
        */
       /*ethan.xu 2021.9.7添加，新增一个机头控制,start*/
        if(0 == add)
        {
            while (sq800_info[NOW][out_status].status.str == status_table[OUT_ABNORMAL].str && try_count)
            {
                while (try_count)
                {
                    try_count -- ;
                    if (sq800_info[NOW][Stucking].status.str == status_table[STUCK_EEROR].str)//卡口罩
                    {
                        uart_driver_deal(stuck_data,sizeof(stuck_data));
                        if (sq800_info[NOW][Stucking].status.str == status_table[STUCK_THING_NOMAL].str)//卡口罩恢复正常
                        {
                            break;
                        }
                    }
                    if (sq800_info[NOW][CutError].status.str == status_table[CUT_LOCAL_ERROR].str)//切刀位置错误
                    {
                        uart_driver_deal(cuterr_data,sizeof(cuterr_data));
                        if (sq800_info[NOW][CutError].status.str == status_table[CUT_LOCAL_NORMAL].str)//切刀位置错误恢复正常
                        {
                            break;
                        }
                    }
                }
                if (0 != try_count)
                {
                    uart_driver_deal(out_data,sizeof(out_data));
                }
            }
        }
        else if(1 == add)
        {
            while (sq800_info_one[NOW][out_status].status.str == status_table[OUT_ABNORMAL].str && try_count)
            {
                while (try_count)
                {
                    try_count -- ;
                    if (sq800_info_one[NOW][Stucking].status.str == status_table[STUCK_EEROR].str)//卡口罩
                    {
                        uart_driver_deal(stuck_data_one,sizeof(stuck_data_one));
                        if (sq800_info_one[NOW][Stucking].status.str == status_table[STUCK_THING_NOMAL].str)//卡口罩恢复正常
                        {
                            break;
                        }
                    }
                    if (sq800_info_one[NOW][CutError].status.str == status_table[CUT_LOCAL_ERROR].str)//切刀位置错误
                    {
                        uart_driver_deal(cuterr_data_one,sizeof(cuterr_data_one));
                        if (sq800_info_one[NOW][CutError].status.str == status_table[CUT_LOCAL_NORMAL].str)//切刀位置错误恢复正常
                        {
                            break;
                        }
                    }
                }
                if (0 != try_count)
                {
                    uart_driver_deal(out_data_one,sizeof(out_data_one));
                }
            }
        }
        else if(2 == add)
        {
            while (sq800_info_two[NOW][out_status].status.str == status_table[OUT_ABNORMAL].str && try_count)
            {
                while (try_count)
                {
                    try_count -- ;
                    if (sq800_info_two[NOW][Stucking].status.str == status_table[STUCK_EEROR].str)//卡口罩
                    {
                        uart_driver_deal(stuck_data_two,sizeof(stuck_data_two));
                        if (sq800_info_two[NOW][Stucking].status.str == status_table[STUCK_THING_NOMAL].str)//卡口罩恢复正常
                        {
                            break;
                        }
                    }
                    if (sq800_info_two[NOW][CutError].status.str == status_table[CUT_LOCAL_ERROR].str)//切刀位置错误
                    {
                        uart_driver_deal(cuterr_data_two,sizeof(cuterr_data_two));
                        if (sq800_info_two[NOW][CutError].status.str == status_table[CUT_LOCAL_NORMAL].str)//切刀位置错误恢复正常
                        {
                            break;
                        }
                    }
                }
                if (0 != try_count)
                {
                    uart_driver_deal(out_data_two,sizeof(out_data_two));
                }
            }
        }
        else if(3 == add)
        {
            while (sq800_info_three[NOW][out_status].status.str == status_table[OUT_ABNORMAL].str && try_count)
            {
                while (try_count)
                {
                    try_count -- ;
                    if (sq800_info_three[NOW][Stucking].status.str == status_table[STUCK_EEROR].str)//卡口罩
                    {
                        uart_driver_deal(stuck_data_three,sizeof(stuck_data_three));
                        if (sq800_info_three[NOW][Stucking].status.str == status_table[STUCK_THING_NOMAL].str)//卡口罩恢复正常
                        {
                            break;
                        }
                    }
                    if (sq800_info_three[NOW][CutError].status.str == status_table[CUT_LOCAL_ERROR].str)//切刀位置错误
                    {
                        uart_driver_deal(cuterr_data_three,sizeof(cuterr_data_three));
                        if (sq800_info_three[NOW][CutError].status.str == status_table[CUT_LOCAL_NORMAL].str)//切刀位置错误恢复正常
                        {
                            break;
                        }
                    }
                }
                if (0 != try_count)
                {
                    uart_driver_deal(out_data_three,sizeof(out_data_three));
                }
            }
        }
        /*ethan.xu 2021.9.7添加，新增一个机头控制,end*/
    }
    printf("single_thing_out end========================[%ld]\n",time(NULL));
}

void HexToStr(uint8_t *hexSrc,char *destStr, int hexLen)
{
    char ddl, ddh;
    int i;
    for (i = 0; i < hexLen; i++)
    {
        ddh = 48 + hexSrc[i] / 16;
        ddl = 48 + hexSrc[i] % 16;
        if (ddh > 57)
            ddh = ddh + 7;
        if (ddl > 57)
            ddl = ddl + 7;
        destStr[i * 2] = ddh;
        destStr[i * 2 + 1] = ddl;
    }
    destStr[hexLen * 2] = '\0';
}

/*
接收串口数据
*/
void uart_receive_thread(void *param)
{
    uint8_t *data = (uint8_t *)malloc(1024);
    while (1)
    {
        printf("5min[%s]5min\n",__FUNCTION__);
        printf("[thread]uart_receive_thread!\n");
        memset(data, 0, 1024);
        int i = 0;
        int lenth = user_uart_receive(data,1023);
        for (i = 0; i < lenth; i++)
        {
            printf("r[%d][%02X] ", i,data[i]);
        }
        if (uart2driver_queue_id.max_depth > 0)
        {
            if (true == check_data_legality(data, lenth))//crc校验
            {
                /*解析串口数据 更新设备状态*/
                if(true == update_device_status(data))
                {
                    uint8_t *tmp = message_queue_message_alloc_blocking(&uart2driver_queue_id);
                    *tmp = data[0];
                    message_queue_write(&uart2driver_queue_id,tmp);
                }
                else
                {
                    post_back_t post_back = {0};
                    post_back.result = false;
                    char tmpStr[50] = {0};
                    HexToStr(data,tmpStr,lenth);
                    snprintf(post_back.debug,50,"UNKNOWN_REPLY[%s]",tmpStr);
                    user_app_reply_func(&post_back);
                }
            }
            else
            {
                post_back_t post_back = {0};
                post_back.result = status_table[DATA_FORMAT_ERROR].value.value_b;
                strcpy(post_back.debug, status_table[DATA_FORMAT_ERROR].str);
                user_app_reply_func(&post_back);
            }
        }
    }
}

/*
控制出货线程，入口参数为：app2driver_queue_id
一次出货支持连续出多个口罩;
1、首先检测机头状态；2、再进行出货
*/
void app_receive_thread(void *param)
{
    message_queue_init(&uart2driver_queue_id, sizeof(uint8_t), 15);
    init_cmd_param();
    while (1)
    {
        uint8_t add;
        printf("5min[%s]5min\n",__FUNCTION__);
        printf("[thread]app_receive_thread!\n");
        post_back_t *post_back = message_queue_read((msg_queue_thread *)param);

        idle_status = false;
        UnSleep_MS(200);//等待机头状态检测线程，检测完成

        for (post_back->current = 1; post_back->current <= post_back->total; post_back->current++)
        {
            /*ethan.xu 2021.9.7添加，新增,4个机头控制,start*/
            if((post_back->num > 0) && (post_back->num <= MASK_DRIVER_NUM))
            {
                add = post_back->num - 1;
            }
            else
            {
                break;/*终止出货*/
            }
            /*ethan.xu 2021.9.7添加，新增,4个机头控制,end*/
            printf("app_receive_thread call check_status_report\n");
            while(busy_flag)//2021.10.18 ethan add
            {
                UnSleep_MS(200);
                printf("check_status_report busy[%ld]=====================\n",time(NULL));
            }
            bool result = check_status_report(add);//返回机头有无回消息

            /*ethan.xu 2021.9.7注释
            if (sq800_info[NOW][CutError].status.str == status_table[CUT_LOCAL_ERROR].str || 
                sq800_info[NOW][Nothing].status.str == status_table[HAVE_NOTHING].str || 
                sq800_info[NOW][Stucking].status.str == status_table[STUCK_EEROR].str )
            {
                sq800_info[NOW][out_status].status = status_table[OUT_ABNORMAL];
            }
            else if(true == result)
            {
                single_thing_out();//出货
            }

            post_back->result = sq800_info[NOW][out_status].status.value.value_b;
            strcpy(post_back->debug, sq800_info[NOW][msg_debug].status.str);
            user_app_reply_func(post_back);//回复服务器

            if (sq800_info[NOW][out_status].status.str == status_table[OUT_ABNORMAL].str)
            {
                break;//终止出货
            }
            */
           if(0 == add)
           {
                if(sq800_info[NOW][CutError].status.str == status_table[CUT_LOCAL_ERROR].str || 
                    sq800_info[NOW][Nothing].status.str == status_table[HAVE_NOTHING].str || 
                    sq800_info[NOW][Stucking].status.str == status_table[STUCK_EEROR].str )
                {
                    sq800_info[NOW][out_status].status = status_table[OUT_ABNORMAL];
                }
                else if(true == result)
                {
                    single_thing_out(add);//出货
                }

                post_back->result = sq800_info[NOW][out_status].status.value.value_b;
                strcpy(post_back->debug, sq800_info[NOW][msg_debug].status.str);
                user_app_reply_func(post_back);//回复服务器

                if (sq800_info[NOW][out_status].status.str == status_table[OUT_ABNORMAL].str)
                {
                    break;/*终止出货*/
                }
           }
           else if(1 == add)
           {
                if(sq800_info_one[NOW][CutError].status.str == status_table[CUT_LOCAL_ERROR].str || 
                    sq800_info_one[NOW][Nothing].status.str == status_table[HAVE_NOTHING].str || 
                    sq800_info_one[NOW][Stucking].status.str == status_table[STUCK_EEROR].str )
                {
                    sq800_info_one[NOW][out_status].status = status_table[OUT_ABNORMAL];
                }
                else if(true == result)
                {
                    single_thing_out(add);//出货
                }

                post_back->result = sq800_info_one[NOW][out_status].status.value.value_b;
                strcpy(post_back->debug, sq800_info_one[NOW][msg_debug].status.str);
                user_app_reply_func(post_back);//回复服务器

                if (sq800_info_one[NOW][out_status].status.str == status_table[OUT_ABNORMAL].str)
                {
                    break;/*终止出货*/
                }
           }
           else if(2 == add)
           {
                if(sq800_info_two[NOW][CutError].status.str == status_table[CUT_LOCAL_ERROR].str || 
                    sq800_info_two[NOW][Nothing].status.str == status_table[HAVE_NOTHING].str || 
                    sq800_info_two[NOW][Stucking].status.str == status_table[STUCK_EEROR].str )
                {
                    sq800_info_two[NOW][out_status].status = status_table[OUT_ABNORMAL];
                }
                else if(true == result)
                {
                    single_thing_out(add);//出货
                }

                post_back->result = sq800_info_two[NOW][out_status].status.value.value_b;
                strcpy(post_back->debug, sq800_info_two[NOW][msg_debug].status.str);
                user_app_reply_func(post_back);//回复服务器

                if (sq800_info_two[NOW][out_status].status.str == status_table[OUT_ABNORMAL].str)
                {
                    break;/*终止出货*/
                }
           }
           else if(3 == add)
           {
                if(sq800_info_three[NOW][CutError].status.str == status_table[CUT_LOCAL_ERROR].str || 
                    sq800_info_three[NOW][Nothing].status.str == status_table[HAVE_NOTHING].str || 
                    sq800_info_three[NOW][Stucking].status.str == status_table[STUCK_EEROR].str )
                {
                    sq800_info_three[NOW][out_status].status = status_table[OUT_ABNORMAL];
                }
                else if(true == result)
                {
                    single_thing_out(add);//出货
                }

                post_back->result = sq800_info_three[NOW][out_status].status.value.value_b;
                strcpy(post_back->debug, sq800_info_three[NOW][msg_debug].status.str);
                user_app_reply_func(post_back);//回复服务器

                if (sq800_info_three[NOW][out_status].status.str == status_table[OUT_ABNORMAL].str)
                {
                    break;/*终止出货*/
                }
           }
        }
        idle_status = true;
        message_queue_message_free((msg_queue_thread *)param, post_back);
    }
}

/*
10s检测一次机头状态
如果当前机头状态为MOTOR_TIMEOUT_ERROR，说明没有改机头，则不进行检测
*/
void idle_request_thread(void *param)
{
    uint8_t check_add = 0;

    while (NET_STATUS_OK!=get_app_network_status())
    {
        UnSleep_MS(100);
    }
    while (1)
    {
        printf("5min[%s]5min\n",__FUNCTION__);
        if (true == idle_status)//出货时不检查状态
        {
            //check_status_report();//ethan.xu 2021.9.7注释
            /*ethan.xu 2021.9.7添加，新增,4个机头控制,start*/
            printf("idle_request_thread call check_status_report\n");
            switch(check_add)
            {
                case 0:
                    if(sq800_info[NOW][msg_debug].status.str != status_table[MOTOR_TIMEOUT_ERROR].str)
                    {
                        check_status_report(check_add);
                    }
                break;
                case 1:
                    if(sq800_info_one[NOW][msg_debug].status.str != status_table[MOTOR_TIMEOUT_ERROR].str)
                    {
                        check_status_report(check_add);
                    }
                break;
                case 2:
                    if(sq800_info_two[NOW][msg_debug].status.str != status_table[MOTOR_TIMEOUT_ERROR].str)
                    {
                        check_status_report(check_add);
                    }
                break;
                case 3:
                    if(sq800_info_three[NOW][msg_debug].status.str != status_table[MOTOR_TIMEOUT_ERROR].str)
                    {
                        check_status_report(check_add);
                    }
                break;
                default:
                break;
            }
            
            check_add++;
            if(check_add >= MASK_DRIVER_NUM)
            {
                check_add = 0;
            }
            /*ethan.xu 2021.9.7添加，新增,4个机头控制,end*/
        }
        UnSleep_S(10);
    }
}
/*
1F 0F 00 01 05 00 34 A2
*/
#elif (USER_APP_TYPE==USER_APP_NEW_MASK)

#include "main_app/hardware_tool/user_uart.h"
#include "user_app.h"

#define CUTTER_NUM_MAX 4
#define MQX_MK_2020_25_MATCH_TABLE_LEN_MAX 8
#define ADD_NUM_MAX 15
volatile bool busy_flag = false;//2021.10.18 ethan add
volatile uint8_t CurrentAdd = 0;

/*框架相关全局静态变量*/
static msg_queue_thread uart2driver_queue_id = {0};

/*业务相关全局静态变量*/
volatile bool idle_status = true;
//static uint8_t idle_data[] = {0x1F, 0x0F, 0x0F, 0x01, 0x05, 0x00, 0xFF, 0xFF};//错误状态查询
static uint8_t check_for_masks[] = {0x1F, 0x0F, 0x0F, 0x01, 0x01, 0x00};//有无口罩查询
static uint8_t check_cut_status[] = {0x1F, 0x0F, 0x0F, 0x01, 0x03, 0x00};//切刀出错状态查询
static uint8_t out_data[] = {0x1F, 0x0F, 0x0F, 0x04, 0x01, 0x01, 0x04};//出票命令
static uint8_t stuck_data[] = {0x1F, 0x0F, 0x0F, 0x05, 0x02, 0x00};//恢复口罩卡住错误状态
static uint8_t cuterr_data[] = {0x1F, 0x0F, 0x0F, 0x05, 0x01, 0x00};//恢复切刀初始位置

/*ethan.xu 2021.9.7添加，新增一个机头控制,start*/
//static uint8_t idle_data_one[] = {0x1F, 0x0F, 0x0E, 0x01, 0x05, 0x00, 0xFF, 0xFF};//读设备传感器状态
static uint8_t check_for_masks_one[] = {0x1F, 0x0F, 0x0E, 0x01, 0x01, 0x00};//有无口罩查询
static uint8_t check_cut_status_one[] = {0x1F, 0x0F, 0x0E, 0x01, 0x03, 0x00};//切刀出错状态查询
static uint8_t out_data_one[] = {0x1F, 0x0F, 0x0E, 0x04, 0x01, 0x01, 0x04};//出票命令
static uint8_t stuck_data_one[] = {0x1F, 0x0F, 0x0E, 0x05, 0x02, 0x00};//恢复口罩卡住错误状态
static uint8_t cuterr_data_one[] = {0x1F, 0x0F, 0x0E, 0x05, 0x01, 0x00};//恢复切刀初始位置

//static uint8_t idle_data_two[] = {0x1F, 0x0F, 0x0D, 0x01, 0x05, 0x00, 0xFF, 0xFF};//读设备传感器状态
static uint8_t check_for_masks_two[] = {0x1F, 0x0F, 0x0D, 0x01, 0x01, 0x00};//有无口罩查询
static uint8_t check_cut_status_two[] = {0x1F, 0x0F, 0x0D, 0x01, 0x03, 0x00};//切刀出错状态查询
static uint8_t out_data_two[] = {0x1F, 0x0F, 0x0D, 0x04, 0x01, 0x01, 0x04};//出票命令
static uint8_t stuck_data_two[] = {0x1F, 0x0F, 0x0D, 0x05, 0x02, 0x00};//恢复口罩卡住错误状态
static uint8_t cuterr_data_two[] = {0x1F, 0x0F, 0x0D, 0x05, 0x01, 0x00};//恢复切刀初始位置

//static uint8_t idle_data_three[] = {0x1F, 0x0F, 0x0C, 0x01, 0x05, 0x00, 0xFF, 0xFF};//读设备传感器状态
static uint8_t check_for_masks_three[] = {0x1F, 0x0F, 0x0C, 0x01, 0x01, 0x00};//有无口罩查询
static uint8_t check_cut_status_three[] = {0x1F, 0x0F, 0x0C, 0x01, 0x03, 0x00};//切刀出错状态查询
static uint8_t out_data_three[] = {0x1F, 0x0F, 0x0C, 0x04, 0x01, 0x01, 0x04};//出票命令
static uint8_t stuck_data_three[] = {0x1F, 0x0F, 0x0C, 0x05, 0x02, 0x00};//恢复口罩卡住错误状态
static uint8_t cuterr_data_three[] = {0x1F, 0x0F, 0x0C, 0x05, 0x01, 0x00};//恢复切刀初始位置
/*ethan.xu 2021.9.7添加，新增一个机头控制,end*/

/*业务相关宏定义 与数据表相关*/
#define STATUS_TABLE_LEN 13
#define INFO_TABLE_TOTAL_LEN 6
#define INFO_TABLE_REPORT_LEN 4
#define MATCH_TABLE_LEN 13
#define TIMEOUT_ALARM_COUNT 120

/*
机头状态，常量
*/
static status_table_t status_table[STATUS_TABLE_LEN] = {
    [OUT_ABNORMAL] = {"OUT_ABNORMAL", false},
    [OUT_NORMAL] = {"OUT_NORMAL", true},
    [MOTOR_TIMEOUT_ERROR] = {"MOTOR_TIMEOUT_ERROR", 1},
    [MOTOR_TIMEOUT_NORMAL] = {"MOTOR_TIMEOUT_NORMAL", 2},
    [DATA_FORMAT_ERROR] = {"DATA_FORMAT_ERROR", false},
    [HAVE_NOTHING] = {"HAVE_NOTHING", 1},
    [HAVE_THING] = {"HAVE_THING", 2},
    [CUT_LOCAL_ERROR] = {"CUT_LOCAL_ERROR", 1},
    [CUT_LOCAL_NORMAL] = {"CUT_LOCAL_NORMAL", 2},
    [STUCK_EEROR] = {"STUCK_EEROR", 1},
    [STUCK_THING_NOMAL] = {"STUCK_THING_NOMAL", 2},
    [BLACK_FLAG_ERROR] = {"BLACK_FLAG_ERROR", 1},
    [STATUS_NULL] = {"STATUS_NULL", 2},
};

/*
* 刀头状态记录，需要参与报警
*/
static sq800_info_table_t MQX_MK_2020_25_InfoOld[CUTTER_NUM_MAX][INFO_TABLE_TOTAL_LEN] = {};
static sq800_info_table_t MQX_MK_2020_25_InfoNew[CUTTER_NUM_MAX][INFO_TABLE_TOTAL_LEN] = {
                            {
                                [CutError] = {"CutError", {"STATUS_NULL", 2}},
                                [Nothing] = {"Nothing", {"STATUS_NULL", 2}},
                                [Stucking] = {"Stucking", {"STATUS_NULL", 2}},
                                [TimeOut] = {"TimeOut", {"STATUS_NULL", 2}},

                                /*下面的状态信息 不参与报警*/
                                [out_status] = {"out_status", {"STATUS_NULL", 2}},
                                [msg_debug] = {"msg_debug", {"STATUS_NULL", 2}},
                            },
                            {
                                [CutError] = {"CutError", {"STATUS_NULL", 2}},
                                [Nothing] = {"Nothing", {"STATUS_NULL", 2}},
                                [Stucking] = {"Stucking", {"STATUS_NULL", 2}},
                                [TimeOut] = {"TimeOut", {"STATUS_NULL", 2}},

                                /*下面的状态信息 不参与报警*/
                                [out_status] = {"out_status", {"STATUS_NULL", 2}},
                                [msg_debug] = {"msg_debug", {"STATUS_NULL", 2}},
                            },
                            {
                                [CutError] = {"CutError", {"STATUS_NULL", 2}},
                                [Nothing] = {"Nothing", {"STATUS_NULL", 2}},
                                [Stucking] = {"Stucking", {"STATUS_NULL", 2}},
                                [TimeOut] = {"TimeOut", {"STATUS_NULL", 2}},

                                /*下面的状态信息 不参与报警*/
                                [out_status] = {"out_status", {"STATUS_NULL", 2}},
                                [msg_debug] = {"msg_debug", {"STATUS_NULL", 2}},
                            },
                            {
                                [CutError] = {"CutError", {"STATUS_NULL", 2}},
                                [Nothing] = {"Nothing", {"STATUS_NULL", 2}},
                                [Stucking] = {"Stucking", {"STATUS_NULL", 2}},
                                [TimeOut] = {"TimeOut", {"STATUS_NULL", 2}},

                                /*下面的状态信息 不参与报警*/
                                [out_status] = {"out_status", {"STATUS_NULL", 2}},
                                [msg_debug] = {"msg_debug", {"STATUS_NULL", 2}},
                            }
};

/*
刀头返回数据帧各字节数据说明，状态类型仅用到0x01-返回读传感器状态，0x04-返回出票命令，0x05-返回错误恢复。
此数组元素名称与实际字节序不一致，byte4-7对应实际byte3-6。
状态1(有无口罩)，byte6 00-有纸，01-无纸。
状态4(出口罩)，
状态5(错误恢复)，type-01恢复切刀错误，02恢复口罩卡住错误。data-使用type的值
仅有 有无口罩、出货正常、出货卡口罩、出货切刀错误
*/
MQX_MK_2020_25_match_table_t MQX_MK_2020_25_MatchTable[MQX_MK_2020_25_MATCH_TABLE_LEN_MAX] = {
                            //byte3-status;byte6-data;错误信息；
                            {0x01, 0x00, HAVE_THING},
                            {0x01, 0x01, HAVE_NOTHING},
                            {0x04, 0x00, OUT_NORMAL},
                            {0x04, 0x01, HAVE_NOTHING},
                            {0x04, 0x02, STUCK_EEROR},
                            {0x04, 0x03, CUT_LOCAL_ERROR},
                            {0x05, 0x01, CUT_LOCAL_NORMAL},
                            {0x05, 0x02, STUCK_THING_NOMAL}
};

/*****************************************************************************
Function:	Updata_MQX_MK_2020_25_InfoNew()
Description:
Input:      add-刀头地址；err_type：刀头上报错误类型
Output:	    no
Return:	    no
Others:     add在调用前已做过转换
History: <author>     <time>	    <desc>	
*****************************************************************************/
void Updata_MQX_MK_2020_25_InfoNew(uint8_t add, uint8_t err_type)
{    
    switch(err_type)
    {
        case HAVE_THING:
        case HAVE_NOTHING:
            MQX_MK_2020_25_InfoNew[add][Nothing].status = status_table[err_type];
            if(HAVE_NOTHING == err_type)
            {
                MQX_MK_2020_25_InfoNew[add][msg_debug].status = status_table[err_type];
                MQX_MK_2020_25_InfoNew[add][out_status].status = status_table[OUT_ABNORMAL];
            }
        break;
        case OUT_NORMAL:
            printf("update_out_status1\n");
            MQX_MK_2020_25_InfoNew[add][out_status].status = status_table[err_type];
            MQX_MK_2020_25_InfoNew[add][msg_debug].status = status_table[err_type];
        break;
        case STUCK_EEROR:
            MQX_MK_2020_25_InfoNew[add][Stucking].status = status_table[err_type];
            MQX_MK_2020_25_InfoNew[add][msg_debug].status = status_table[err_type];
            MQX_MK_2020_25_InfoNew[add][out_status].status = status_table[OUT_ABNORMAL];
        break;
        case CUT_LOCAL_ERROR:
        case CUT_LOCAL_NORMAL:
            printf("laile 4\n");
            MQX_MK_2020_25_InfoNew[add][CutError].status = status_table[err_type];
            if(CUT_LOCAL_ERROR == err_type)
            {
                MQX_MK_2020_25_InfoNew[add][msg_debug].status = status_table[err_type];
                MQX_MK_2020_25_InfoNew[add][out_status].status = status_table[OUT_ABNORMAL];
            }
        break;
        case STUCK_THING_NOMAL:
            printf("laile 3\n");
            MQX_MK_2020_25_InfoNew[add][Stucking].status = status_table[err_type];
        break;
        default:
        break;
    }
}

/*****************************************************************************
Function:	update_device_status()
Description:接收到刀头返回数据后，更新刀头状态
Input:      刀头回复数据
Output:	    no
Return:	    no
Others:     true-协议内数据；false-未知数据。
History: <author>     <time>	    <desc>	
*****************************************************************************/
bool update_device_status(uint8_t *data)
{
    uint8_t i = 0, j = 0;
    uint8_t add;

    add = ADD_NUM_MAX - data[2];
    printf("update_device_status add %d\n",add);
    for(i=0; i<MQX_MK_2020_25_MATCH_TABLE_LEN_MAX; i++)
    {
        if(0x05 != data[2])//有无口罩状态查询/出口罩
        {
            if((data[3] == MQX_MK_2020_25_MatchTable[i].byte3) && (data[6] == MQX_MK_2020_25_MatchTable[i].byte6))
            {
                Updata_MQX_MK_2020_25_InfoNew(add, MQX_MK_2020_25_MatchTable[i].uindex);
                return true;
            }
        }
        else//错误恢复
        {
            if(i >= 6)
            {
                printf("laile\n");
                if((data[2] == MQX_MK_2020_25_MatchTable[i].byte3) && (data[3] == MQX_MK_2020_25_MatchTable[i].byte6))
                {
                    printf("laile 2\n");
                    Updata_MQX_MK_2020_25_InfoNew(CurrentAdd, MQX_MK_2020_25_MatchTable[i].uindex);
                    return true;
                }
            }
        }
    }
    return false;
}

uint16_t crc16_xmode_custom(uint8_t *data, uint32_t lenth)
{
    uint16_t wCRCin = 0x0000;
    uint16_t wCPoly = 0x8005;
    uint8_t i = 0, j = 0;
    for (j = 0; j < lenth; j++)
    {
        for (i = 0; i < 8; i++)
        {
            uint8_t bit_value = ((data[j] >> (7 - i)) & 0x01);
            uint8_t c15_value = ((wCRCin >> 15) & 0x01);
            wCRCin = wCRCin << 1;
            if (bit_value != c15_value)
            {
                wCRCin = wCRCin ^ wCPoly;
            }
        }
    }
    wCRCin = wCRCin & 0xFFFF;
    return (wCRCin ^ 0x0000);
}

/*
校验串口接收到的数据
*/
static bool check_data_legality(uint8_t *data, uint32_t lenth)
{
    if (0x1F != data[0] || 0x0F != data[1])
    {
        return false;
    }
    else
    {
        uint16_t crc_result = crc16_xmode_custom(data, lenth - 2);
        if (data[lenth - 2] != (crc_result >> 8) || data[lenth - 1] != (crc_result & 0xFF))
        {
            return false;
        }
    }
    return true;
}

/*****************************************************************************
Function:	uart_driver_deal()
Description:发送串口消息，等待机头恢复串口消息(有软件超时)，超时后标记超时错误。更新设备状态信息(仅不报警那块)
Input:      no
Output:	    no
Return:	    true机头正常返回数据，false机头串口通信超时
Others:     地址范围会转为0-3
History: <author>     <time>	    <desc>	
*****************************************************************************/
static bool uart_driver_deal(uint8_t *data, uint32_t lenth)
{
    uint8_t i = 0;
    static uint32_t timeout_count = 0;
    uint8_t add = ADD_NUM_MAX - data[2];

    for (i = 0; i < 2; i++)
    {
        // UnSleep_MS(500); /* 非常重要!!! 为从机释放总线留足时间*/
        printf("uart_driver_deal start[%d]=======================[%ld]\n",i,time(NULL));
        user_uart_send(0, data, lenth);
        uint8_t *data_byte = message_queue_timeout(&uart2driver_queue_id,50);//等5s
        if (NULL != data_byte)//机头响应
        {
            printf("uart_driver_deal end=======================[%ld]\n",time(NULL));
            timeout_count = 0;
            message_queue_message_free(&uart2driver_queue_id,data_byte);
            return true;
        }
        else
        {
            timeout_count++;
            printf("uart_driver_deal timeout[%d]=======================[%ld]\n",timeout_count,time(NULL));
            if (timeout_count>=TIMEOUT_ALARM_COUNT)
            {
                timeout_count = 0;
                MQX_MK_2020_25_InfoNew[add][TimeOut].status = status_table[MOTOR_TIMEOUT_ERROR];
            }
        }
    }
    printf("update_out_status2\n");
    MQX_MK_2020_25_InfoNew[add][out_status].status = status_table[OUT_ABNORMAL];
    MQX_MK_2020_25_InfoNew[add][msg_debug].status = status_table[MOTOR_TIMEOUT_ERROR];

    return false;
}

void add_crc16_param(uint8_t *data,uint8_t len)
{
    uint16_t crc16_result = crc16_xmode_custom(data, len - 2);
    data[len-2] = crc16_result >> 8;
    data[len-1] = crc16_result & 0xFF;
}

/*
初始化下发参数
*/
void init_cmd_param(void)
{
    // add_crc16_param(idle_data,sizeof(idle_data));
    // add_crc16_param(out_data,sizeof(out_data));
    add_crc16_param(stuck_data,sizeof(stuck_data));
    add_crc16_param(cuterr_data,sizeof(cuterr_data));

    /*ethan.xu 2021.9.7添加，新增一个机头控制,start*/
    // add_crc16_param(idle_data_one,sizeof(idle_data_one));
    // add_crc16_param(out_data_one,sizeof(out_data_one));
    add_crc16_param(stuck_data_one,sizeof(stuck_data_one));
    add_crc16_param(cuterr_data_one,sizeof(cuterr_data_one));

    // add_crc16_param(idle_data_two,sizeof(idle_data_two));
    // add_crc16_param(out_data_two,sizeof(out_data_two));
    add_crc16_param(stuck_data_two,sizeof(stuck_data_two));
    add_crc16_param(cuterr_data_two,sizeof(cuterr_data_two));

    // add_crc16_param(idle_data_three,sizeof(idle_data_three));
    // add_crc16_param(out_data_three,sizeof(out_data_three));
    add_crc16_param(stuck_data_three,sizeof(stuck_data_three));
    add_crc16_param(cuterr_data_three,sizeof(cuterr_data_three));
    /*ethan.xu 2021.9.7添加，新增一个机头控制,end*/
}


/*****************************************************************************
Function:	check_status_report()
Description:检测机头状态并上报
            读取刀头当前有无口罩。
Input:      add:刀头地址，0-3
Output:	    no
Return:	    true机头回了消息，false机头未回消息
Others:     no
History: <author>     <time>	    <desc>	
*****************************************************************************/
bool check_status_report(uint8_t add)
{
    uint8_t i = 0;
    bool result;

    busy_flag = true;
    printf("check_status_report start[%ld]=====================\n",time(NULL));

    /*ethan.xu 2021.9.7添加，新增一个机头控制,start*/
    cJSON *array = cJSON_CreateArray();

    result = uart_driver_deal(check_for_masks, sizeof(check_for_masks));//阻塞读设备传感器状态

    for (i = 0; i < INFO_TABLE_REPORT_LEN; i++)//报警状态
    {
        if(MQX_MK_2020_25_InfoOld[add][i].status.str != MQX_MK_2020_25_InfoNew[add][i].status.str)
        {
            MQX_MK_2020_25_InfoOld[add][i].status = MQX_MK_2020_25_InfoNew[add][i].status;
            cJSON *item = cJSON_CreateObject();
            create_json_table(item, J_Int, "digital", add);
            create_json_table(item, J_String, "key", MQX_MK_2020_25_InfoNew[add][i].str);
            create_json_table(item, J_Int, "status", MQX_MK_2020_25_InfoNew[add][i].status.value.value_c);
            create_json_table(array, J_Array, item);
        }
    }

    /*ethan.xu 2021.9.7添加，新增一个机头控制,end*/

    if (cJSON_GetArraySize(array)>0)
    {
        user_report_alarm_func(array);//3816
    }else
    {
        cJSON_Delete(array);
    }
    printf("check_status_report end[%ld]=====================\n",time(NULL));
    busy_flag = false;

    return result;
}


/*****************************************************************************
Function:	single_thing_out()
Description:出货
Input:      add:刀头地址，0-3
Output:	    no
Return:	    no
Others:     出货前更新出货状态为正常出货，出货后再次更新出货状态
History: <author>     <time>	    <desc>	
*****************************************************************************/
void single_thing_out(uint8_t add)
{
    printf("single_thing_out start========================[%ld]\n",time(NULL));
    uint8_t try_count = 3;
    bool ret;

    printf("update_out_status3\n");
    MQX_MK_2020_25_InfoNew[add][out_status].status = status_table[OUT_NORMAL];
    CurrentAdd = add;
    if(0 == add)
    {
        ret = uart_driver_deal(out_data,sizeof(out_data));
    }
    else if(1 == add)
    {
        ret = uart_driver_deal(out_data_one,sizeof(out_data_one));
    }
    else if(2 == add)
    {
        ret = uart_driver_deal(out_data_two,sizeof(out_data_two));
    }
    else if(3 == add)
    {
        ret = uart_driver_deal(out_data_three,sizeof(out_data_three));
    }

    if(ret)
    {
        while(MQX_MK_2020_25_InfoNew[add][out_status].status.str == status_table[OUT_ABNORMAL].str && try_count)
        {
            printf("err huifu\n");
            while(try_count)
            {
                try_count -- ;
                if (MQX_MK_2020_25_InfoNew[add][Stucking].status.str == status_table[STUCK_EEROR].str)//卡口罩
                {
                    printf("Stucking err huifu\n");
                    uart_driver_deal(stuck_data,sizeof(stuck_data));
                    printf("Stucking err%d %s\n",add,MQX_MK_2020_25_InfoNew[add][Stucking].status.str);
                    printf("Stucking err %s\n",status_table[STUCK_THING_NOMAL].str);
                    if (MQX_MK_2020_25_InfoNew[add][Stucking].status.str == status_table[STUCK_THING_NOMAL].str)//卡口罩恢复正常
                    {
                        break;
                    }
                }
                if (MQX_MK_2020_25_InfoNew[add][CutError].status.str == status_table[CUT_LOCAL_ERROR].str)//切刀位置错误
                {
                    printf("CutError err huifu\n");
                    uart_driver_deal(cuterr_data,sizeof(cuterr_data));
                    if (MQX_MK_2020_25_InfoNew[add][CutError].status.str == status_table[CUT_LOCAL_NORMAL].str)//切刀位置错误恢复正常
                    {
                        break;
                    }
                }
            }
            if (0 != try_count)
            {
                if(0 == add)
                {
                    uart_driver_deal(out_data,sizeof(out_data));
                }
                else if(1 == add)
                {
                    uart_driver_deal(out_data_one,sizeof(out_data_one));
                }
                else if(2 == add)
                {
                    uart_driver_deal(out_data_two,sizeof(out_data_two));
                }
                else if(3 == add)
                {
                    uart_driver_deal(out_data_three,sizeof(out_data_three));
                }
            }
        }
    }
    printf("single_thing_out end========================[%ld]\n",time(NULL));
}

void HexToStr(uint8_t *hexSrc,char *destStr, int hexLen)
{
    char ddl, ddh;
    int i;
    for (i = 0; i < hexLen; i++)
    {
        ddh = 48 + hexSrc[i] / 16;
        ddl = 48 + hexSrc[i] % 16;
        if (ddh > 57)
            ddh = ddh + 7;
        if (ddl > 57)
            ddl = ddl + 7;
        destStr[i * 2] = ddh;
        destStr[i * 2 + 1] = ddl;
    }
    destStr[hexLen * 2] = '\0';
}

/*
接收串口数据，更新刀头状态
*/
void uart_receive_thread(void *param)
{
    uint8_t *data = (uint8_t *)malloc(1024);
    while (1)
    {
        printf("[thread]uart_receive_thread!\n");
        memset(data, 0, 1024);
        int i = 0;
        int lenth = user_uart_receive(data,1023);
        for (i = 0; i < lenth; i++)
        {
            printf("r[%d][%02X] ", i,data[i]);
        }
        if (uart2driver_queue_id.max_depth > 0)
        {
            // if (true == check_data_legality(data, lenth))//crc校验
            // {
                /*解析串口数据 更新设备状态*/
                if(true == update_device_status(data))
                {
                    uint8_t *tmp = message_queue_message_alloc_blocking(&uart2driver_queue_id);
                    *tmp = data[0];
                    message_queue_write(&uart2driver_queue_id,tmp);
                }
                else
                {
                    post_back_t post_back = {0};
                    post_back.result = false;
                    char tmpStr[50] = {0};
                    HexToStr(data,tmpStr,lenth);
                    snprintf(post_back.debug,50,"UNKNOWN_REPLY[%s]",tmpStr);
                    user_app_reply_func(&post_back);
                }
            // }
            // else//校验出问题
            // {
            //     post_back_t post_back = {0};
            //     post_back.result = status_table[DATA_FORMAT_ERROR].value.value_b;
            //     strcpy(post_back.debug, status_table[DATA_FORMAT_ERROR].str);
            //     user_app_reply_func(&post_back);
            // }
        }
    }
}

/*****************************************************************************
Function:	app_receive_thread()
Description:控制出货线程（mqtt）
Input:      app2driver_queue_id
Output:	    no
Return:	    no
Others:     一次出货支持连续出多个口罩;
1、首先检测机头状态；2、再进行出货
History: <author>     <time>	    <desc>	
*****************************************************************************/
void app_receive_thread(void *param)
{
    message_queue_init(&uart2driver_queue_id, sizeof(uint8_t), 15);
    //init_cmd_param();
    while (1)
    {
        uint8_t add;
        printf("[thread]app_receive_thread!\n");
        post_back_t *post_back = message_queue_read((msg_queue_thread *)param);

        idle_status = false;
        UnSleep_MS(200);//等待机头状态检测线程，检测完成

        for (post_back->current = 1; post_back->current <= post_back->total; post_back->current++)
        {
            if((post_back->num > 0) && (post_back->num <= CUTTER_NUM_MAX))
            {
                add = post_back->num - 1;
            }
            else
            {
                break;/*终止出货*/
            }

            printf("app_receive_thread call check_status_report\n");
            while(busy_flag)//2021.10.18 ethan add，错开10s定时和机头通信
            {
                UnSleep_MS(200);
                printf("check_status_report busy[%ld]=====================\n",time(NULL));
            }
            bool result = check_status_report(add);//返回机头有无回消息

            if(MQX_MK_2020_25_InfoNew[add][CutError].status.str == status_table[CUT_LOCAL_ERROR].str || 
                MQX_MK_2020_25_InfoNew[add][Nothing].status.str == status_table[HAVE_NOTHING].str || 
                MQX_MK_2020_25_InfoNew[add][Stucking].status.str == status_table[STUCK_EEROR].str )
            {
                printf("update_out_status4\n");
                MQX_MK_2020_25_InfoNew[add][out_status].status = status_table[OUT_ABNORMAL];
            }
            else if(true == result)
            {
                single_thing_out(add);//出货
            }

            printf("update_out_status5\n");
            post_back->result = MQX_MK_2020_25_InfoNew[add][out_status].status.value.value_b;
            strcpy(post_back->debug, MQX_MK_2020_25_InfoNew[add][msg_debug].status.str);
            user_app_reply_func(post_back);//回复服务器

            if (MQX_MK_2020_25_InfoNew[add][out_status].status.str == status_table[OUT_ABNORMAL].str)
            {
                break;/*终止出货*/
            }
        }
        idle_status = true;
        message_queue_message_free((msg_queue_thread *)param, post_back);
    }
}

/*****************************************************************************
Function:	idle_request_thread()
Description:10s检测一次机头状态
Input:      app2driver_queue_id
Output:	    no
Return:	    no
Others:     如果当前机头状态为MOTOR_TIMEOUT_ERROR，说明没有该机头，则不进行检测
History: <author>     <time>	    <desc>	
*****************************************************************************/
void idle_request_thread(void *param)
{
    uint8_t check_add = 0;

    while (NET_STATUS_OK!=get_app_network_status())
    {
        UnSleep_MS(100);
    }
    while (1)
    {
        if (true == idle_status)//出货时不检查状态
        {
            printf("idle_request_thread call check_status_report\n");

            if(MQX_MK_2020_25_InfoNew[check_add][msg_debug].status.str != status_table[MOTOR_TIMEOUT_ERROR].str)
            {
                check_status_report(check_add);
            }
            
            check_add++;
            if(check_add >= CUTTER_NUM_MAX)
            {
                check_add = 0;
            }
        }
        UnSleep_S(10);
    }
}
#endif
