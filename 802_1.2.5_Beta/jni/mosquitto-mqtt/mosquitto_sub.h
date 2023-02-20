/*
 * mosquitto_sub.h
 *
 *  Created on: 2020年12月31日
 *      Author: Administrator
 */

#ifndef JNI_MOSQUITTO_MQTT_MOSQUITTO_SUB_H_
#define JNI_MOSQUITTO_MQTT_MOSQUITTO_SUB_H_


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>
#include <getopt.h>
#include <string.h>
#include "mosquitto/mosquitto.h"
#include <stdlib.h>

#include "cJSON.h"

#define  MQTT_PORT     1883
#define  MQTT_USER     "admin"
#define  MQTT_PASS     "password"

#define MSG_MAX_SIZE  512

#define  SUBQOS        1
#define  KEEPALIVE     30

#ifdef __cplusplus
extern "C"
{
#endif

void publishMsg(const char *msg);
int startMosquittoSub(char *imei, char *subTopic, char *pubTopic);

#ifdef __cplusplus
};
#endif

void connect_callback(struct mosquitto *mosq, void *obj, int rc);
void message_callback(struct mosquitto *mosq, void* obj, const struct mosquitto_message *message);
void *MqttThread();


#endif /* JNI_MOSQUITTO_MQTT_MOSQUITTO_SUB_H_ */
