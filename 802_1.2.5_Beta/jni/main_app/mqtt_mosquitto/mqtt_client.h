#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#define TOPIC_MAX_COUNT 15

typedef int (*cbFnc_t)(char *,int);
void mqtt_client_setup(const char *host,int tPorts, const char *clientId);
void mqtt_setparam(int keepAlive,const char *User,const char *Passwd,int cleanSession,const char *will);
int mqtt_client_subscribe(const char *topic ,int qos);
int mqtt_client_publish(const char *topic ,int qos,const char *msg,int lenth);
int register_on(const char *evt,cbFnc_t cbFnc);


#endif/*MQTT_CLIENT_H*/

