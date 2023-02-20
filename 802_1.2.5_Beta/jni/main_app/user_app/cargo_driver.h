#ifndef CARGO_DRIVER_H
#define CARGO_DRIVER_H

#include "main_app/main_app.h"
#if (USER_APP_TYPE==USER_APP_CARGO)
#include "main_app/hardware_tool/user_uart.h"
#include "main_app/cJSON_file/json_app.h"

typedef struct 
{
    char seq_id[50];
    char cmd_id[50];
    char debug[50];
    bool result;
    int cmd;
    int current;
    int total;
    int num;
}post_back_t;

void uart_receive_thread(void *param);
void app_receive_thread(void *param);
void RequestMcuInfo(char **version);
#endif 
#endif//CARGO_DRIVER_H
