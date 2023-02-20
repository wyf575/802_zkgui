#ifndef MASK_DRIVER_H
#define MASK_DRIVER_H

#include <stdint.h>
#include "main_app/main_app.h"
#if (USER_APP_TYPE==USER_APP_MASK)
/*框架相关结构体*/
typedef struct 
{
    char seq_id[50];
    char cmd_id[50];
    char debug[50];
    bool result;
    int cmd;
    int current;
    int total;
    int num;/*货道号*/
}post_back_t;

/*业务相关结构体*/
typedef struct
{
    const char *str;
    union
    {
        uint8_t value_c;
        bool value_b;
    } value;
} status_table_t;
typedef struct
{
    const char *str;
    status_table_t status;
} sq800_info_table_t;

typedef struct
{
    uint8_t byte4;
    uint8_t byte5;
    uint8_t byte7;
    const uint8_t uindex[6];
} match_table_t;

typedef enum
{
    NOW = 0,
    LAST = 1,
} table_enum;

typedef enum
{
    OUT_ABNORMAL = 0,/*出口罩失败*/
    OUT_NORMAL,/*出口罩正常*/
    MOTOR_TIMEOUT_ERROR,
    MOTOR_TIMEOUT_NORMAL,
    DATA_FORMAT_ERROR,/*机头回复数据格式错误*/
    HAVE_NOTHING,/*无口罩*/
    HAVE_THING,
    CUT_LOCAL_ERROR,/*切刀位置错误*/
    CUT_LOCAL_NORMAL,/*切刀位置错误回复正常*/
    STUCK_EEROR,/*口罩卡住*/
    STUCK_THING_NOMAL,/*口罩卡住恢复正常*/
    BLACK_FLAG_ERROR,/*黑标错误*/
    STATUS_NULL,
} status_enum;

typedef enum
{
    CutError = 0,/*切刀错误*/
    Nothing ,/*缺口罩*/
    Stucking ,/*卡口罩*/
    TimeOut ,/*设备失联*/
    out_status ,
    msg_debug ,
} info_enum;

/*框架相关API声明*/
void app_receive_thread(void *param);
void uart_receive_thread(void *param);

/*业务相关API声明*/
void idle_request_thread(void *param);

#elif (USER_APP_TYPE==USER_APP_NEW2_MASK)
/*框架相关结构体*/
typedef struct 
{
    char seq_id[50];
    char cmd_id[50];
    char debug[50];
    bool result;
    int cmd;
    int current;
    int total;
    int num;/*货道号*/
}post_back_t;

/*业务相关结构体*/
typedef struct
{
    const char *str;
    union
    {
        uint8_t value_c;
        bool value_b;
    } value;
} status_table_t;
typedef struct
{
    const char *str;
    status_table_t status;
} sq800_info_table_t;

typedef struct
{
    uint8_t byte8;
    uint8_t byte9;
    uint8_t uindex;
} match_table_t;

typedef enum
{
    NOW = 0,
    LAST = 1,
} table_enum;

typedef enum
{
    OUT_ABNORMAL = 0,/*出口罩失败*/
    OUT_NORMAL,/*出口罩正常*/
    MOTOR_TIMEOUT_ERROR,
    MOTOR_TIMEOUT_NORMAL,
    DATA_FORMAT_ERROR,/*机头回复数据格式错误*/
    HAVE_NOTHING,/*无口罩 04*/
    HAVE_THING,
    TICKET_IN_OUTSIDE,/*出票未拿走 出货失败 02*/
    TICKET_NOT_TAKEN,/*出票已送至出口 15s未拿走 成功 03*/
    STUCKING,/*指令发送 票未送至出口 01*/
    STATUS_NULL,
} status_enum;

typedef enum
{
    
    Nothing = 0 ,/*缺口罩*/ 
    TimeOut ,/*设备失联*/
    Ticket_In_Outside,
    Stucking,
    out_status ,
    msg_debug ,
} info_enum;

/*框架相关API声明*/
void app_receive_thread(void *param);
void uart_receive_thread(void *param);

/*业务相关API声明*/
void idle_request_thread(void *param);

#elif (USER_APP_TYPE==USER_APP_NEW_MASK)
/*框架相关结构体*/
typedef struct
{
    char seq_id[50];
    char cmd_id[50];
    char debug[50];
    bool result;
    int cmd;
    int current;
    int total;
    int num;/*货道号*/
}post_back_t;

/*业务相关结构体*/
typedef struct
{
    const char *str;
    union
    {
        uint8_t value_c;
        bool value_b;
    } value;
} status_table_t;
typedef struct
{
    const char *str;
    status_table_t status;
} sq800_info_table_t;

/*刀头状态对比*/
typedef struct
{
    uint8_t byte3;
    uint8_t byte6;
    uint8_t uindex;
} MQX_MK_2020_25_match_table_t;

typedef enum
{
    NOW = 0,
    LAST = 1,
} table_enum;

/*
刀头调试错误
*/
typedef enum
{
    OUT_ABNORMAL = 0,/*出口罩失败*/
    OUT_NORMAL,/*出口罩正常*/
    MOTOR_TIMEOUT_ERROR,
    MOTOR_TIMEOUT_NORMAL,
    DATA_FORMAT_ERROR,/*机头回复数据格式错误*/
    HAVE_NOTHING,/*无口罩*/
    HAVE_THING,
    CUT_LOCAL_ERROR,/*切刀位置错误*/
    CUT_LOCAL_NORMAL,/*切刀位置错误回复正常*/
    STUCK_EEROR,/*口罩卡住*/
    STUCK_THING_NOMAL,/*口罩卡住恢复正常*/
    BLACK_FLAG_ERROR,/*黑标错误*/
    STATUS_NULL,
} status_enum;

/*
服务端定义错误
*/
typedef enum
{
    CutError = 0,/*切刀错误*/
    Nothing ,/*缺口罩*/
    Stucking ,/*卡口罩*/
    TimeOut ,/*设备失联*/
    out_status ,
    msg_debug ,
} info_enum;

/*框架相关API声明*/
void app_receive_thread(void *param);
void uart_receive_thread(void *param);

/*业务相关API声明*/
void idle_request_thread(void *param);

#endif

#endif//MASK_DRIVER_H
