/*
 * mosquitto_sub.c
 *
 *  Created on: 2020年12月31日
 *      Author: Administrator
 */

#include "mosquitto_sub.h"

#include "model/JsonParse.h"
#include <pthread.h>

#include "../net/Module4GManager.h"
#include "appconfig.h"
#include "baselib/UrlManager.h"

struct mosquitto *g_mosq = NULL;
char* curImei;
char* curSubTopic;
char* curPubTopic;

char* getMqttServerUrl() {
	if (IS_TEST_MODE) {
//		return "tcp://tmp.ibeelink.com:1883";
		return "tmp.ibeelink.com";
	} else {
		return "mqtt.ibeelink.com";
	}
}

void printJson(cJSON * root)//以递归的方式打印json的最内层键值对
{
    for(int i=0; i < cJSON_GetArraySize(root); i++)   //遍历最外层json键值对
    {
        cJSON * item = cJSON_GetArrayItem(root, i);
        if(cJSON_Object == item->type)      //如果对应键的值仍为cJSON_Object就递归调用printJson
            printJson(item);
        else                                //值不为json对象就直接打印出键和值
        {
            printf("%s:", item->string);
            printf("%s\r\n", cJSON_Print(item));
        }
    }
}

void publishMsg(const char *msg)
{
	if(g_mosq == NULL){
		return;
	}
	printf("[MQTT]  publishMsg\n");
	int length = strlen(msg);
	printf("[MQTT]  publishMsg %d - %s - %s\n", length, msg, curPubTopic);
	/*发布消息*/
	mosquitto_publish(g_mosq, NULL, curPubTopic, length, msg, 0, 0);
}

void connect_callback(struct mosquitto *mosq, void *obj, int rc)
{
	printf("mqtt - connect_callback: %d, %s, %s\n", rc, curImei, curSubTopic);
    if(!rc)
    {
        if(mosquitto_subscribe(mosq, NULL, curSubTopic, 0) != MOSQ_ERR_SUCCESS )
        {
            printf("Mosq_subcrible() error: %s\n", strerror(errno));
            mqttConnectStatus(0);
            return ;
        }
        printf("subicrible topic:%s\n", curSubTopic);
        mqttConnectStatus(1);
        return;
    }
    mqttConnectStatus(0);
    return;
}

void message_callback(struct mosquitto *mosq, void* obj, const struct mosquitto_message *message)
{
    cJSON *buf = NULL;
    printf("--------message_callback---------\n");
    printf("subcrible topic is %s\n", message->topic);
    printf("message:  \n");
    paseMqttSubMsg(message->payload);
    return;
}

void subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
    printf("Subscribed (mid: %d): %d\n", mid, granted_qos[0]);
    printf("Subscribed qos_count: %d\n", qos_count);

}

void disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{
	printf("Call the function: my_disconnect_callback\n");
}

void publish_callback(struct mosquitto *mosq, void *obj, int mid)
{
	printf("Call the function: my_publish_callback\n");
}

void *MqttThread(){
	 /*以下为MQTT标准操作*/
	while (1)
	{
	   mosquitto_lib_init();
	   char* clientId = curImei;//MODULE4GMANAGER->getIMEI();
	   printf("clientId=%s\n", clientId);
	   g_mosq = mosquitto_new(clientId, 1, NULL);
	   if (!g_mosq)
	   {
		   printf("MQTT Error: Out of memory.\n");
		   mosquitto_lib_cleanup();
		   continue;
	   }
	   //mosquitto_log_callback_set(g_mosq, my_log_callback);
	   mosquitto_threaded_set(g_mosq, 1);
	   mosquitto_username_pw_set(g_mosq, MQTT_USER, MQTT_PASS);
	   mosquitto_connect_callback_set(g_mosq, connect_callback);
	   mosquitto_message_callback_set(g_mosq, message_callback);
       mosquitto_subscribe_callback_set(g_mosq, subscribe_callback);
       mosquitto_publish_callback_set(g_mosq, publish_callback);
       mosquitto_disconnect_callback_set(g_mosq, disconnect_callback);

//       while (NET_STATUS_OK != get_network_status())
//       {
//           sleep(1);
//       }

	   /*为减轻服务器负担 取出设备ID最后一位作为随机延时 10--20s*/
	   int sleep_s = clientId[14] - 37;
	   sleep(sleep_s);

	   if (mosquitto_connect(g_mosq, getMqttServerUrl(), MQTT_PORT, KEEPALIVE))
	   {
		   printf("MQTT Unable to connect. :%s\n", strerror(errno));
		   continue;
	   }
	   printf("MQTT mosquitto_loop_forever\n");
	   mosquitto_loop_forever(g_mosq, -1, 1);
	   printf("MQTT mosquitto_loop_forever - 1\n");
	   mosquitto_destroy(g_mosq);
	   mosquitto_lib_cleanup();
	}
}

int startMosquittoSub(char* imei, char* subTopic, char* pubTopic)
{
	curImei = imei;
	curSubTopic = subTopic;
	curPubTopic = pubTopic;
	printf("----startMosquittoSub----%s, %s, %s\n", curImei, curSubTopic, curPubTopic);
	pthread_t mqttThread;
	int ret = pthread_create(&mqttThread, NULL, (void*)MqttThread, NULL);
	if(ret){
		printf("[mqttThread] create error\n");
		return 0;
	}else{
		printf("[mqttThread] create success\n");
	}
	return 1;
}

