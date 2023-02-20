#include "main_app/main_app.h"
#include "main_app/net_listener.h"
#include "main_app/upgrade_tool/update_modem.h"
#include "mosquitto.h"
#include "main_app/sntp_client/sntp_client.h"
#include "main_app/message_queue/message_queue.h"
#include "mqtt_client.h"

struct mosquitto *g_mosq_handler = NULL;
static char Host[64] = "mqtt.ibeelink.com";
static char ClientID[200] = "123456789012345";
static char user[64] = "user";
static char password[65] = "passwd";
static int Port = 1883;
static int sKeepAlive = 60;
static int sCleanSession = 1;

typedef struct
{
    char topic[255];
    int qos;
} sub_topic_t;
static sub_topic_t sub_topic[TOPIC_MAX_COUNT] = {0};
typedef struct
{
    char topic[255];
    char msg[2048];
    int lenth;
    int qos;
} topic_msg_t;

typedef struct 
{
    const char *evt;
    cbFnc_t cbFnc;
}evtCb_t;
static evtCb_t *evtCb = NULL;
static int evtindex = 0 ;

static msg_queue_thread sub_topic_queue_id = {0};
static msg_queue_thread pub_topic_queue_id = {0};



static void register_call(const char *evt,char *str,int num)
{
    int i = 0;
    if (NULL == evtCb)
    {
        printf("please register_on[%s] first\n", evt);
        return;
    }
    for ( i = 0; i < evtindex; i++)
    {
        if (!strcmp(evt,(evtCb+i)->evt))
        {
            (evtCb+i)->cbFnc(str,num);
        }
    }
}

/**
* @brief         : 订阅、发布、log回调
* @attention     : 作为调试时查看
* @param[in]	 : 设备信息
* @param[out]    : 全局变量
* @return        : 0
* @author        : aiden.xiang@ibeelink.com
* @date          : 2019.10.28
*/
static void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
    int i;
    printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
    for (i = 1; i < qos_count; i++)
    {
        printf(", %d", granted_qos[i]);
    }
    printf("\n");
}
static void my_publish_callback(struct mosquitto *mosq, void *obj, int mid)
{
    printf("pushlish success!\n");
}
static void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
    /* Pring all log messages regardless of level. */
    printf("================= [%s] ================= \n", str);
}

/**
* @brief         : MQTT断开回调
* @attention     : 更新mqtt连接状态
* @param[in]	 : 设备信息
* @param[out]    : 全局变量
* @return        : 0
* @author        : aiden.xiang@ibeelink.com
* @date          : 2019.10.28
*/
static void my_disconnect_callback(struct mosquitto *mosq, void *userdata, int result)
{
    set_app_network_status(NET_STATUS_FAILE);
    mosquitto_disconnect(mosq);
}
/**
* @brief         : MQTT连接回调
* @attention     : 订阅topic在此处完成
* @param[in]	 : 设备信息
* @param[out]    : 全局变量
* @return        : 0
* @author        : aiden.xiang@ibeelink.com
* @date          : 2019.10.28
* @date          : 2021.05.12
*/
static void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
    if (!result)
    {
        printf("\n$$$$$$$$$$$$$$$$$$ success! $$$$$$$$$$$$$$$$$$$\n");
        set_app_network_status(NET_STATUS_OK);
        for (size_t i = 0; 0 != strlen(sub_topic[i].topic); i++)
        {
            mosquitto_subscribe(mosq,NULL,sub_topic[i].topic, sub_topic[i].qos);
            UnSleep_MS(500);
        }
        register_call("mqtt_connected","connected!",0);
        // update_sntp_time(NTP_SERVER1); /*采用公网NTP*/
    }
    else
    {
        fprintf(stderr, "Connect failed\n");
    }
}
/**
* @brief         : MQTT消息回调
* @attention     : 该消息入队parse_mqtt_data_queue
* @param[in]	 : 设备信息
* @param[out]    : 全局变量
* @return        : 0
* @author        : aiden.xiang@ibeelink.com
* @date          : 2019.10.28
* @date          : 2021.05.12
*/
static void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    if (message->payloadlen)
    {
        // printf("%s %s\n", message->topic, message->payload);
        topic_msg_t *mqtt_msg =  message_queue_message_alloc_blocking(&sub_topic_queue_id);
        mqtt_msg->lenth = message->payloadlen;
        mqtt_msg->msg[message->payloadlen] = '\0';
        memcpy(mqtt_msg->msg, message->payload, message->payloadlen);
        message_queue_write(&sub_topic_queue_id, mqtt_msg);
    }
    else
    {
        printf("%s (null)\n", message->topic);
    }
    fflush(stdout);
}


static void mqtt_client_thread(void *param)
{
    while (1)
    {
        printf("5min[%s]5min\n",__FUNCTION__);
        printf("[thread]mqtt_client_thread!\n");
        while(1)
        {
            if(NET_STATUS_OK == get_network_status())
            {
                break;
            }
            else
            {
                printf("[thread]mqtt_client_thread wait net\n");
                sleep(2);
            }
        }
        register_call("update_callback",Host,ibeelink_updata_function(Host));
        
        set_app_network_status(NET_STATUS_FAILE);
        /*为减轻服务器负担 取出设备ID最后一位作为随机延时 1--10s*/
        UnSleep_S((ClientID[14] - 47)>0?(ClientID[14] - 47):1);
        
        /*mosquitto标准化操作*/
        mosquitto_lib_init();
        g_mosq_handler = mosquitto_new(ClientID, (1 == sCleanSession), NULL);
        if (!g_mosq_handler)
        {
            fprintf(stderr, "Error: Out of memory.\n");
            continue;
        }
        mosquitto_threaded_set(g_mosq_handler, true);
        mosquitto_username_pw_set(g_mosq_handler, user, password);

        /*给MQTT设置各回调*/
        //mosquitto_log_callback_set(g_mosq_handler, my_log_callback);
        mosquitto_connect_callback_set(g_mosq_handler, my_connect_callback);
        mosquitto_message_callback_set(g_mosq_handler, my_message_callback);
        mosquitto_subscribe_callback_set(g_mosq_handler, my_subscribe_callback);
        mosquitto_publish_callback_set(g_mosq_handler, my_publish_callback);
        mosquitto_disconnect_callback_set(g_mosq_handler, my_disconnect_callback);

        /*发起连接且在连接断开后释放资源*/
        if (mosquitto_connect(g_mosq_handler, Host, Port, sKeepAlive))
        {
            fprintf(stderr, "Unable to connect.\n");
            continue;
        }
        printf("mqtt_client_thread connected!!!\n");
        mosquitto_loop_forever(g_mosq_handler, -1, 1);
        mosquitto_destroy(g_mosq_handler);
        mosquitto_lib_cleanup();
        printf("mqtt_client_thread disconnected!!!\n");
    }
}

static void mqtt_publish_thread(void *param)
{
    while (1)
    {
        printf("5min[%s]5min\n",__FUNCTION__);
        printf("[thread]mqtt_publish_thread!\n");
        topic_msg_t *pub_topic = message_queue_read(&pub_topic_queue_id); 
        /*下面做消息发布处理*/
        mosquitto_publish(g_mosq_handler,NULL,pub_topic->topic,pub_topic->lenth,pub_topic->msg, pub_topic->qos, false);
        message_queue_message_free(&pub_topic_queue_id, pub_topic);
        UnSleep_MS(100);
    }
}



static void mqtt_subscribe_thread(void *arg)
{
    while (1)
    {
        printf("5min[%s]5min\n",__FUNCTION__);
        printf("[thread]mqtt_subscribe_thread!\n");
        topic_msg_t *mqtt_msg = message_queue_read(&sub_topic_queue_id); 
        /*下面做消息接收处理*/
        //printf("mqtt_subscribe_thread len[%d] [%s]\n", mqtt_msg->lenth, mqtt_msg->msg);
        register_call("mqtt_receive", mqtt_msg->msg, mqtt_msg->lenth);
        message_queue_message_free(&sub_topic_queue_id, mqtt_msg);
    }
}





/**
 * @brief 对外提供向本文件注册回调
 * 用于连接成功以及消息通知等功能
 * @param evt 回调名称
 * @param cbFnc 回调函数
 * @author aiden.xiang@ibeelink.com
 * @date 2021.05.12
 * @return int 正常：当前回调的排序 错误：0
 */
int register_on(const char *evt,cbFnc_t cbFnc)
{
    if (NULL == evtCb)
    {
        evtCb = (evtCb_t *)malloc(10*sizeof(evtCb_t));
    }
    if (evtindex>=10)
    {
        printf("register_on overflow[%d]\n", evtindex);
        return 0;
    }
    (evtCb+evtindex)->evt = evt;
    (evtCb+evtindex)->cbFnc = cbFnc;
    evtindex = evtindex + 1;
    return evtindex;
}

/**
 * @brief 对外提供消息发布接口
 * 
 * @param topic 消息主题
 * @param qos 消息质量
 * @param msg 消息体
 * @param lenth 消息长度
 * @author aiden.xiang@ibeelink.com
 * @date 2021.05.12
 * @return int 正常：大于0
 */
int mqtt_client_publish(const char *topic, int qos, const char *msg, int lenth)
{
    topic_msg_t *pub_topic =  message_queue_message_alloc_blocking(&pub_topic_queue_id);
    pub_topic->lenth = lenth;
    pub_topic->qos = qos;
    pub_topic->msg[lenth] = '\0';
    strcpy(pub_topic->topic, topic);
    memcpy(pub_topic->msg, msg, lenth);
    message_queue_write(&pub_topic_queue_id, pub_topic);
    return lenth;
}

/**
 * @brief 对外提供订阅主题接口
 * 注意：可以重复调用 但必须在连线前
 * @param topic 订阅主题
 * @param qos 消息质量
 * @author aiden.xiang@ibeelink.com
 * @date 2021.05.12
 * @return int 正常：大于0
 */
int mqtt_client_subscribe(const char *topic, int qos)
{
    static int topic_index = 0;
    if (TOPIC_MAX_COUNT == topic_index)
    {
        return -1;
    }
    strcpy(sub_topic[topic_index].topic, topic);
    sub_topic[topic_index].qos = qos;
    topic_index++;
    return topic_index;
}

/**
 * @brief 对外提供设置MQTT参数
 * 注意：必须在连线前调用 不调用会使用缺省值
 * @param keepAlive 保活时间
 * @param User 用户名
 * @param Passwd 密码
 * @param cleanSession 是否清除会话
 * @param will 遗嘱消息(暂未支持)
 * @author aiden.xiang@ibeelink.com
 * @date 2021.05.12
 */
void mqtt_setparam(int keepAlive, const char *User, const char *Passwd, int cleanSession, const char *will)
{
    strcpy(user, User);
    strcpy(password, Passwd);
    sKeepAlive = keepAlive;
    sCleanSession = cleanSession;
    // if (NULL != will)
    // {
    //     general_mqtt_will((INT8 *)will, 1, false, "DEVICE WILL MSG!");
    // }
}

/**
 * @brief 对外提供MQTT客户端连线
 * 注意：若未提前设置参数则会使用缺省值
 * @param host 域名或IP
 * @param tPorts 端口号
 * @param clientId 客户端ID
 * @author aiden.xiang@ibeelink.com
 * @date 2021.05.12
 */
void mqtt_client_setup(const char *host, int tPorts, const char *clientId)
{
    message_queue_init(&sub_topic_queue_id, sizeof(topic_msg_t), TOPIC_MAX_COUNT);
    message_queue_init(&pub_topic_queue_id, sizeof(topic_msg_t), TOPIC_MAX_COUNT);

    strcpy(Host, host);
    strcpy(ClientID, clientId);
    Port = tPorts;

    register_user_thread(mqtt_client_thread, NULL,NULL);//
    register_user_thread(mqtt_publish_thread, NULL,NULL);//推送线程
    register_user_thread(mqtt_subscribe_thread, NULL,NULL);//订阅线程
}


