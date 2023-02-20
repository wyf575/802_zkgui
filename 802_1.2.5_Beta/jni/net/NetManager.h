/*
 * NetManager.h
 *
 *  Created on: 2021年3月11日
 *      Author: Administrator
 */

#ifndef JNI_NET_NETMANAGER_H_
#define JNI_NET_NETMANAGER_H_


typedef void (*NetListenerCallback)(char* netType);

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

int getNetConnectStatus(char* netType);
int initNetManager(NetListenerCallback callback);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* JNI_NET_NETMANAGER_H_ */
