#include "user_app.h"
#include "main_app/sntp_client/sntp_client.h"

#if (USER_APP_TYPE==USER_APP_CARGO)
#include "main_app/upgrade_tool/update_mcu.h"
#endif

static msg_queue_thread app2driver_queue_id = {0};

/*
解析3812出货指令&回复服务端第0个的状态
*/
int user_app_deal(cJSON *table)
{
    printf("==========================user_app_deal_mqtt==========================\n");
    if (NULL != table)
    {
        post_back_t post_back = {0};
        post_back.total=1;
        post_back.num = 1;
        post_back.cmd = 1000;

        parse_json_table(table,J_String,"seq",post_back.seq_id);
        cJSON *extend = cJSON_GetObjectItem(table,"extend");
        cJSON *input = cJSON_GetObjectItem(extend,"input");
        parse_json_table(extend,J_String,"cmd_id",post_back.cmd_id);
        parse_json_table(extend,J_Int,"cmd",&post_back.cmd);

        cJSON *ibeelink_rsp = cJSON_CreateObject();
        cJSON *extend_json = cJSON_CreateObject();
        cJSON *resultData = cJSON_CreateObject();
        create_json_table(ibeelink_rsp,J_Int,"cmd",4812);
        create_json_table(ibeelink_rsp,J_String,"msg","success");
        create_json_table(ibeelink_rsp,J_String,"seq",post_back.seq_id);

        create_json_table(ibeelink_rsp,J_Item,"extend",extend_json);
        create_json_table(extend_json,J_Int,"cmd",post_back.cmd);
        create_json_table(extend_json,J_Bool,"result",true);
        create_json_table(extend_json,J_Item,"resultData",resultData);
        create_json_table(resultData,J_Int,"current",0);
        ibeelink_send_msg(ibeelink_rsp);
        cJSON_Delete(ibeelink_rsp);

        if (NULL != input)
        {
            parse_json_table(input,J_Int,"digital",&post_back.num);
            parse_json_table(input,J_Int,"count",&post_back.total);
        }
        if (0 != app2driver_queue_id.max_depth)
        {
            post_back_t *send_msg =  message_queue_message_alloc_blocking(&app2driver_queue_id);
            *send_msg = post_back;
            message_queue_write(&app2driver_queue_id, send_msg);
        } 
    }
    return 0;
}
/*上报出货状态*/
void user_app_reply_func(post_back_t *post_back)
{
    cJSON *ibeelink_rsp = cJSON_CreateObject();
    cJSON *extend = cJSON_CreateObject();
    cJSON *resultData = cJSON_CreateObject();
    create_json_table(ibeelink_rsp,J_Int,"cmd",4812);
    create_json_table(ibeelink_rsp,J_String,"msg","success");
    create_json_table(ibeelink_rsp,J_String,"seq",post_back->seq_id);
    create_json_table(ibeelink_rsp,J_Long,"digital",get_system_time_stamp(NULL));
    create_json_table(ibeelink_rsp,J_Item,"extend",extend);
    create_json_table(extend,J_Int,"cmd",post_back->cmd);
    create_json_table(extend,J_String,"cmd_id",post_back->cmd_id);
    create_json_table(extend,J_Bool,"result",post_back->result);
    create_json_table(extend,J_Item,"resultData",resultData);
    create_json_table(resultData,J_Int,"current",post_back->current);
    create_json_table(resultData,J_Int,"total",post_back->total);
    create_json_table(resultData,J_String,"rec_cmd",post_back->debug);
    ibeelink_send_msg(ibeelink_rsp);
    cJSON_Delete(ibeelink_rsp);
}
/*上报报警状态*/
void user_report_alarm_func(cJSON *extend)
{
    cJSON *ibeelink_rsp = cJSON_CreateObject();
    create_json_table(ibeelink_rsp,J_Int,"cmd",3816);
    create_json_table(ibeelink_rsp,J_String,"msg","success");
    create_json_table(ibeelink_rsp,J_Long,"digital",get_system_time_stamp(NULL));
    create_json_table(ibeelink_rsp,J_Item,"extend",extend);
    ibeelink_send_msg(ibeelink_rsp);
    cJSON_Delete(ibeelink_rsp);
}

int update_callback(char *server,int status)
{
    printf("================= update_callback[%s][%s][%d] =================\n",MAIN_APP_VERSION,server,status);
    
    /*
    1、存储内容: 当前版本号----时间----版本类型
    2、存储时机: 请求到有升级包时
    3、操作逻辑：每次启动后 比较当前版本号与存储版本号 若有更新 再比较时间小于7天 则闪烁OTA指示灯
    */
    return 0;
}

void start_user_app(void)
{
    message_queue_init(&app2driver_queue_id, sizeof(post_back_t), TOPIC_MAX_COUNT);
    #if (USER_APP_TYPE==USER_APP_CARGO)
        ibeelink_mqtt_init(update_mcu_callback);
        register_user_thread(mcu_update_task, NULL, NULL);
    #else
        ibeelink_mqtt_init(update_callback);
        register_user_thread(idle_request_thread, NULL, NULL);
    #endif
    register_user_thread(uart_receive_thread, NULL, NULL);
    ibeelink_mqtt_callback(user_app_deal);
	register_user_thread(app_receive_thread, (void *)&app2driver_queue_id,NULL);
}
