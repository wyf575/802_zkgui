#ifndef IBEELINK_MQTT_H
#define IBEELINK_MQTT_H

#include "../main_app.h"
#include "../mqtt_mosquitto/mqtt_client.h"
#include "../cJSON_file/json_app.h"


typedef int (*mqtt_user_callback)(cJSON *);//函数指针

void ibeelink_mqtt_callback(mqtt_user_callback user_app_deal_mqtt);
void ibeelink_mqtt_init(cbFnc_t update_callback);
int ibeelink_send_msg(cJSON *send_table);

#endif/*IBEELINK_MQTT_H*/

