/**
 * @file ibeelink_mqtt.c
 * @author your name (you@domain.com)
 * @brief 注意：未完成AES加密模式 因此注释了相关函数
 * @version 0.1
 * @date 2021-05-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "../main_app.h"
#include "../mqtt_mosquitto/mqtt_client.h"
#include "../cJSON_file/json_app.h"
#include "../md5_tool/md5.h"
#include "../cJSON_file/read_json_file.h"
#include "../base64_tool/base64.h"
#include "../debug_printf/debug_printf.h"
#include "../net_listener.h"
#include "../ibeelink_app/ibeelink_mqtt.h"
#include "../ibeelink_app/ibeelink_http.h"
#include "../device_info.h"
#include "../message_queue/message_queue.h"
#include "../sntp_client/sntp_client.h"
#include "../linux_tool/busy_box.h"

extern int get_ad_and_config_params(void);

#define FUNC_MAX 15
#define IBEELINK_KEY_FILE "ibeelink_key.asf"

extern void sysReboot(void);

typedef void (*pfunc)(cJSON *json_table);
static char report_topic[100] = {0};
static char emit_topic[100] = {0};
static char ibeelink_now[100] = {0};
static int aes_mode = 0;
static msg_queue_thread receive_queue_id = {0};

int ibeelink_send_msg(cJSON *send_table)
{
    int cmd_num = 0;
    char *send_buf = (char *)malloc(1024 * sizeof(char));
    memset(send_buf,0,1024);
    char *json_str = cJSON_PrintUnformatted(send_table);
    if (1 == aes_mode)
    {
        // char *encode_aes = (char *)malloc(1024 * sizeof(char));
        // char *encode_base64 = (char *)malloc(1024 * sizeof(char));
        // memset(encode_aes,0,1024);
        // memset(encode_base64,0,1024);
        // int lenth = aes_ecb_encode(ibeelink_now,json_str,strlen(json_str),encode_aes);
        // base64_encode(encode_aes,lenth,encode_base64,NULL);
        // snprintf(send_buf,1024,"$$$%s",encode_base64);
        // free(encode_aes);
        // free(encode_base64);
        base64_encode(json_str,strlen(json_str),send_buf,NULL);
    }else
    {
        base64_encode(json_str,strlen(json_str),send_buf,NULL);
    }
    debuglog_yellow("sendBuf[%s]\n",json_str);
    parse_json_table(send_table,J_Int,"cmd",&cmd_num);
    if (4812 == cmd_num)
    {
        /*做队列管理 使用cmdid匹配5812 发送4812后需要等待上一个5812解锁*/
        char *cmd_id  = (char *)malloc(50);
        memset(cmd_id,0,50);
        cJSON *extern_json = cJSON_GetObjectItem(send_table,"extend");
        parse_json_table(extern_json,J_String,"cmd_id",cmd_id);
        mqtt_client_publish(report_topic,0,send_buf,strlen(send_buf));
        free(cmd_id);
    }else
    {
        mqtt_client_publish(report_topic,0,send_buf,strlen(send_buf));
    }
    free(send_buf);
    free(json_str);
    return 0;
}

static void use_3702_test(char *json_string)
{
    // cJSON *ibeelink_rsp = cJSON_CreateObject();
    // cJSON *flag_json = cJSON_CreateObject();

    // create_json_table(ibeelink_rsp,J_Int,"cmd",3702);
    // create_json_table(ibeelink_rsp,J_String,"msg","request");
    // create_json_table(ibeelink_rsp,J_Long,"digital",get_system_time_stamp(NULL));
    // create_json_table(ibeelink_rsp,J_Item,"extend",flag_json);
    // create_json_table(flag_json,J_String,"flag",json_string);
    // char *aesencode = (char *)malloc(1024);
    // char *base64encode = (char *)malloc(1024);
    // char *send_buf = (char *)malloc(1024);
    // memset(aesencode,0,1024);
    // memset(base64encode,0,1024);
    // char *json_buf = cJSON_PrintUnformatted(ibeelink_rsp);
    // printf("************************use_3702_test[%s][%s]\n",json_string,ibeelink_now);
    // int lenth = aes_ecb_encode(ibeelink_now,json_buf,strlen(json_buf),aesencode);
    // base64_encode(aesencode,lenth,base64encode,NULL);
    // snprintf(send_buf,1024,"$$$%s",base64encode);
    // mqtt_client_publish(report_topic,0,send_buf,strlen(send_buf));
    // free(json_buf);
    // free(send_buf);
    // free(base64encode);
    // free(aesencode);
    // cJSON_Delete(ibeelink_rsp);
}

void deal_3801_func(cJSON *json_table)
{
    cJSON *ibeelink_rsp = cJSON_CreateObject();
    cJSON *extend = cJSON_CreateObject();
    create_json_table(ibeelink_rsp,J_Int,"cmd",3801);
    create_json_table(ibeelink_rsp,J_String,"msg","success");
    create_json_table(ibeelink_rsp,J_Item,"extend",extend);
    create_json_table(extend,J_String,"v",MAIN_APP_VERSION);
    create_json_table(extend,J_String,"imei",UnGetDeviceId());
    create_json_table(extend,J_String,"iccid",UnGetICCId());
    create_json_table(extend,J_Int,"sig",UnGetCsqValue());
    create_json_table(extend,J_String,"baseLoc",UnGetLBSInfo());
    create_json_table(extend,J_Int,"netMode",4);
    ibeelink_send_msg(ibeelink_rsp);
    cJSON_Delete(ibeelink_rsp);
    get_ad_and_config_params();
}

static void deal_3802_func(cJSON *json_table)
{
    cJSON *ibeelink_rsp = cJSON_CreateObject();
    create_json_table(ibeelink_rsp,J_Int,"cmd",4802);
    create_json_table(ibeelink_rsp,J_String,"msg","pong");
    create_json_table(ibeelink_rsp,J_Long,"digital",get_system_time_stamp(NULL));
    ibeelink_send_msg(ibeelink_rsp);
    //device_register(IBEELINK_SERVER,"deal_3802_func",4802);
    cJSON_Delete(ibeelink_rsp);
}

static void deal_3803_func(cJSON *json_table)
{
    cJSON *ibeelink_rsp = cJSON_CreateObject();
    create_json_table(ibeelink_rsp,J_Int,"cmd",4803);
    create_json_table(ibeelink_rsp,J_String,"msg","success");
    create_json_table(ibeelink_rsp,J_Long,"digital",get_system_time_stamp(NULL));
    ibeelink_send_msg(ibeelink_rsp);
    printf("server[3803] need reboot system!\n");
    cJSON_Delete(ibeelink_rsp);
    sysReboot();
}

static void deal_3900_func(cJSON *json_table)
{
    char *print_json = cJSON_Print(json_table);
    aes_mode = 0;
    printf("deal_3900_func[%s]\n",print_json);
    free(print_json);
}

static void deal_4800_func(cJSON *json_table)
{
    char *print_json = cJSON_Print(json_table);
    printf("deal_3900_func[%s]\n",print_json);
    free(print_json);
}

static void deal_3901_func(cJSON *json_table)
{
    /*服务器端安全模式密钥校验失败 取出之前的密钥作为当前密钥*/
    read_pairs_file("last",ibeelink_now,IBEELINK_KEY_FILE);
    save_pairs_file("now",ibeelink_now,IBEELINK_KEY_FILE);
}

static void deal_4702_func(cJSON *json_table)
{
    // char *tmp_key = (char *)malloc(100);
    // memset(tmp_key,0,100);
    // parse_json_table(json_table,J_String,"msg",tmp_key);
    // if (strlen(tmp_key)>0)
    // {
    //     strcpy(ibeelink_now,tmp_key);
    //     save_pairs_file("last",ibeelink_now,IBEELINK_KEY_FILE);
    //     use_3702_test("test");
    // }else if (cJSON_HasObjectItem(json_table,"extend"))
    // {
    //     cJSON *extend = cJSON_GetObjectItem(json_table,"extend");
    //     memset(tmp_key,0,100);
    //     parse_json_table(extend,J_String,"flag",tmp_key);
    //     if (!strcmp(tmp_key,"test"))
    //     {
    //         printf(" warning:aes_key[%s] updated!!!!!!!!! \n",ibeelink_now);
    //         save_pairs_file("now",ibeelink_now,IBEELINK_KEY_FILE);
    //         use_3702_test("");
    //     }else
    //     {
    //         read_pairs_file("now",ibeelink_now,IBEELINK_KEY_FILE);
    //     }
    // }
    // free(tmp_key);
}

static void deal_3701_func(cJSON *json_table)
{
    // cJSON *ibeelink_rsp = cJSON_CreateObject();
    // create_json_table(ibeelink_rsp,J_Int,"cmd",4701);
    // create_json_table(ibeelink_rsp,J_String,"msg","success");
    // create_json_table(ibeelink_rsp,J_Long,"digital",get_system_time_stamp(NULL));
    // create_json_table(ibeelink_rsp,J_String,"extend","");
    // ibeelink_send_msg(ibeelink_rsp);
    // use_3702_test("");
    // cJSON_Delete(ibeelink_rsp);
}

static void deal_3703_func(cJSON *json_table)
{
    // cJSON *ibeelink_rsp = cJSON_CreateObject();
    // create_json_table(ibeelink_rsp,J_Int,"cmd",4703);
    // create_json_table(ibeelink_rsp,J_String,"msg","ok");
    // create_json_table(ibeelink_rsp,J_Long,"digital",get_system_time_stamp(NULL));
    // create_json_table(ibeelink_rsp,J_String,"extend","");
    // parse_json_table(json_table,J_String,"msg",ibeelink_now);
    // save_pairs_file("now",ibeelink_now,IBEELINK_KEY_FILE);
    // ibeelink_send_msg(ibeelink_rsp);
    // use_3702_test("test");
    // cJSON_Delete(ibeelink_rsp);
}
static void deal_5812_func(cJSON *json_table)
{
    /*
    sys.publish("SEND_4812",tjsondata["extend"]["cmd_id"]
    */
}

static void get_ibeelink_aes_key(void)
{
    // char *tmp_key = (char *)malloc(100);
    // memset(tmp_key,0,100);
    // read_pairs_file("now",tmp_key,IBEELINK_KEY_FILE);
    // if (strlen(tmp_key)>0)
    // {
    //     strcpy(ibeelink_now,tmp_key);
    // }else
    // {
    //     char sub_imei[15] = {0};
    //     char sub_imei_md5[50] = {0};
    //     strncpy(sub_imei,UnGetDeviceId(),10);
    //     CalculateMD5(sub_imei,sub_imei_md5,sizeof(sub_imei_md5),0);
    //     strcpy(ibeelink_now,sub_imei_md5);
    //     save_pairs_file("now",sub_imei_md5,IBEELINK_KEY_FILE);
    //     save_pairs_file("last",sub_imei_md5,IBEELINK_KEY_FILE);
    // }
    // free(tmp_key);
}

static int mqtt_msg_decode(char *originStr,char *aes_key,char *outdata)
{
    if (originStr[0]=='$'&&originStr[1]=='$'&&originStr[2]=='$')
    {
        // char tmp_aes_key[50] = {0};
        // char *aes_encode = (char *)malloc(1024);
        // int outlen = 0;
        // aes_mode = 1;
        // memset(aes_encode,0,1024);
        // if (originStr[3]=='$')
        // {
        //     char sub_imei[15] = {0};
        //     strncpy(sub_imei,UnGetDeviceId()+5,10);
        //     CalculateMD5(sub_imei,tmp_aes_key,sizeof(tmp_aes_key),0);
        //     base64_decode(originStr+4,strlen(originStr)-4,aes_encode,&outlen);
        // }else
        // {
        //     strcpy(tmp_aes_key,aes_key);
        //     base64_decode(originStr+3,strlen(originStr)-3,aes_encode,&outlen);
        // }
        // aes_ecb_decode(tmp_aes_key,aes_encode,outlen,outdata);
        // return aes_mode;//加密模式下解析
    }else
    {
        aes_mode = 0;
        base64_decode(originStr,strlen(originStr),(unsigned char *)outdata,NULL);
        return aes_mode;//普通模式下解析
    }
    return 0;
}

static void deal_3807_func(cJSON *json_table)
{
    cJSON *ibeelink_rsp = cJSON_CreateObject();
    create_json_table(ibeelink_rsp,J_Int,"cmd",4807);
    create_json_table(ibeelink_rsp,J_String,"msg","response");
    create_json_table(ibeelink_rsp,J_Long,"digital",get_system_time_stamp(NULL));
    create_json_table(ibeelink_rsp,J_String,"extend","");
    ibeelink_send_msg(ibeelink_rsp);
    cJSON_Delete(ibeelink_rsp);
    get_ad_and_config_params();
}

static pfunc deal_func[FUNC_MAX+1]={
    deal_5812_func,
    deal_3801_func,
    deal_3802_func,
    deal_3803_func,
    deal_3900_func,
    deal_4800_func,
    deal_3901_func,
    deal_4702_func,
    deal_3701_func,
    deal_3703_func,
    deal_3807_func,
    NULL,
};

static uint8_t match_index(uint32_t num)
{
	switch (num)
	{
	case 5812:return 0;
	case 3801:return 1;
	case 3802:return 2;
	case 3803:return 3;
	case 3900:return 4;
	case 4800:return 5;
	case 3901:return 6;
	case 4702:return 7;
	case 3701:return 8;
	case 3703:return 9;
	case 3807:return 10;
	}
    return FUNC_MAX;
}

static void ibeelink_deal_thread(void *arg)
{
    message_queue_init(&receive_queue_id, 2048, 20);
    while (1)
    {
        printf("5min[%s]5min\n",__FUNCTION__);
        printf("[thread]ibeelink_deal_thread!\n");
        char decode_data[2048] = {0};
        char *recv_data = message_queue_read(&receive_queue_id); 
        debuglog_yellow("ibeelink_deal_thread recv_data=========[%s]\n",recv_data);
        int ret = mqtt_msg_decode(recv_data,ibeelink_now,decode_data);
        cJSON *json_table = cJSON_Parse(decode_data);
        if (1 == ret)
        {
            if (NULL == json_table)
            {
                char last_key[50] = {0};
                read_pairs_file("last",last_key,IBEELINK_KEY_FILE);
                mqtt_msg_decode(recv_data,last_key,decode_data);
                json_table = cJSON_Parse(decode_data);
            }
        }
        debuglog_yellow("ibeelink_deal_thread decode_data=========[%s]\n",decode_data);
        if (NULL != json_table)
        {
            static char last_seq[50] = {0}; 
            int cmd_num = 0,i = 0;
            char seq_value[50] = {0};
            parse_json_table(json_table,J_String,"seq",seq_value);
            if (0 == strlen(seq_value) || (strcmp(seq_value,last_seq)))
            {
                strcpy(last_seq,seq_value);
                parse_json_table(json_table,J_Int,"cmd",&cmd_num);
                printf("ibeelink_deal_thread[%d]\n",cmd_num);
                if (3812 == cmd_num && NULL != arg)
                {
                    printf("(*(mqtt_user_callback*)arg)(json_table)\n");
                    (*((mqtt_user_callback )arg))(json_table);
                }else if (NULL != deal_func[match_index(cmd_num)])
                {
                    deal_func[match_index(cmd_num)](json_table);
                }
            }
        }
        if(NULL != json_table)
        {
            cJSON_Delete(json_table);
        }
        message_queue_message_free(&receive_queue_id, recv_data);
    }
}

static int mqtt_connected(char *msg,int lenth)
{
    deal_3801_func(NULL);
    printf("mqtt_connected!!!!!!!!!!\n");
    return 0;
}

static int mqtt_receive(char *recv_data,int lenth)
{
    debuglog_yellow("mqtt_receive!!!!!!!!!![%s]\n",recv_data);
    char *message_data =  message_queue_message_alloc_blocking(&receive_queue_id);
    memcpy(message_data, recv_data, lenth);
    message_data[lenth]='\0';
    message_queue_write(&receive_queue_id, message_data);
    return lenth;
}

/**
 * @brief 对外提供注册APP消息回调的接口
 * 
 * @param user_app_deal_mqtt 回调函数
 */
void ibeelink_mqtt_callback(mqtt_user_callback user_app_deal_mqtt)
{
    register_user_thread(ibeelink_deal_thread,(void *)user_app_deal_mqtt,NULL);
}

/**
 * @brief 
 * 
 * @param update_callback 升级接口
 */
void ibeelink_mqtt_init(cbFnc_t update_callback)
{
    while (NET_STATUS_OK != get_network_status())
    {
        UnSleep_S(1);
    } 
    snprintf(report_topic,100,"/ibeelink/device/report/%s",UnGetMD5DeviceId());
    snprintf(emit_topic,100,"/ibeelink/device/emit/%s",UnGetMD5DeviceId());
    //debuglog_yellow("report_topic[%s] emit_topic[%s]\n",report_topic,emit_topic);
    // get_ibeelink_aes_key();
    mqtt_client_subscribe(emit_topic,0);
    if (NULL != update_callback)
    {
        register_on("update_callback",update_callback);
    }
    register_on("mqtt_receive",mqtt_receive);
    register_on("mqtt_connected",mqtt_connected);
    mqtt_client_setup(IBEELINK_SERVER,1883,UnGetDeviceId());
}
