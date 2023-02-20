/*
 * DeviceSigManager.cpp
 *
 *  Created on: 2021年2月19日
 *      Author: Administrator
 */

#include "DeviceSigManager.h"

#include "../net/Module4GManager.h"
#include "utils/log.h"
#include "appconfig.h"
#include "net/NetManager.h"

static int wificonnStatus = 0;
static int cur_dBm = 0;

DeviceSigManager::DeviceSigManager(){
}

DeviceSigManager* DeviceSigManager::getInstance(){
	static DeviceSigManager mInstance;
	return &mInstance;
}


void DeviceSigManager::WifiConnStatusCallback(char *pSsid, int status, int quality){
	wificonnStatus = status;

	if (quality <= 0)
		cur_dBm = -100;
	else if (quality >= 100)
		cur_dBm = -50;
	else
		cur_dBm = (quality / 2) - 100;

	LOGD("wifi连接回调 status=%d , dBm=%d\n", wificonnStatus, cur_dBm);
}

void DeviceSigManager::WifiSignalStatusCallback(ScanResult_t *pstScanResult, int resCnt){
//	if(wificonnStatus){
		if(resCnt > 0){
			cur_dBm = pstScanResult[0].signalSTR;
//			LOGD("wifi信号更新 dBm=%d\n", cur_dBm);
		}
//	}
}

int DeviceSigManager::getNetType(){
	if(getNetConnectStatus("eth0")){
		return NETWORKTYPE_ETHERNET;
	}else if(getNetConnectStatus("wlan0")){
		return NETWORKTYPE_WIFI;
	}else if(getNetConnectStatus("eth2")){
		return NETWORKTYPE_4G;
	}else{
		return NETWORKTYPE_NONE;
	}
}

DeviceSigManager::S_NetSig* DeviceSigManager::getNetSignal(){
	int type = getNetType();
	S_NetSig* s_netSig = (S_NetSig*) malloc(sizeof(S_NetSig));
	LOGD("------getNetSignal %d\n", type);
	s_netSig->sigType = type;
	LOGD("1------getNetSignal %d\n", type);
	switch (type) {
		case NETWORKTYPE_ETHERNET:
			s_netSig->sig = 31;
			break;
		case NETWORKTYPE_WIFI:
			if(cur_dBm >= -55 && cur_dBm <= 0){ // 最强 24 - 31
				if(cur_dBm >= -48){
					s_netSig->sig = 31;
				}else{
					s_netSig->sig = 24 + cur_dBm + 55;
				}
				break;
			}else if(cur_dBm <= -100){ // 微弱 0 - 7
				if(cur_dBm <= -107){
					s_netSig->sig = 0;
				}else{
					s_netSig->sig = 7 + 100 + cur_dBm;
				}
				break;

			}else{ // 较强 较弱 8 - 23
				s_netSig->sig = (int) (7 + (-55 - cur_dBm) * 16/45.0);
				break;
			}
		case NETWORKTYPE_4G:
			LOGD("1------NETWORKTYPE_4G\n");
			s_netSig->sig = MODULE4GMANAGER->get4GSig();
			break;
		default:
			s_netSig->sig = 99;
			break;
	}
	return s_netSig;
}

void DeviceSigManager::updateSigUI(ZKTextView* sigView){
	S_NetSig* s_netSig = getNetSignal();
	LOGD("----updateSigUI---%d\n", sigView);
	switch(s_netSig->sigType){
		case NETWORKTYPE_ETHERNET:
			sigView->setBackgroundPic("sig/stat_sys_eth_connected.png");
		break;
		case NETWORKTYPE_4G:
			LOGD("----updateSigUI---NETWORKTYPE_4G %d\n", s_netSig->sig);
			if(s_netSig->sig < 2){
				sigView->setBackgroundPic("sig/sig_4g_0.png");
			}else if(s_netSig->sig == 99){
				sigView->setBackgroundPic("sig/no_sig.png");
			}else{
				int level = s_netSig->sig / 7;
				switch (level) {
					case 0:
						sigView->setBackgroundPic("sig/sig_4g_1.png");
						break;
					case 1:
						sigView->setBackgroundPic("sig/sig_4g_2.png");
						break;
					case 2:
						sigView->setBackgroundPic("sig/sig_4g_3.png");
						break;
					case 3:
						sigView->setBackgroundPic("sig/sig_4g_4.png");
						break;
					default:
						break;
				}
			}
		break;
		case NETWORKTYPE_WIFI:
			int level = s_netSig->sig / 6;
			switch (level) {
				case 0:
					sigView->setBackgroundPic("sig/sig_wifi_0.png");
					break;
				case 1:
					sigView->setBackgroundPic("sig/sig_wifi_1.png");
					break;
				case 2:
					sigView->setBackgroundPic("sig/sig_wifi_2.png");
					break;
				case 3:
					sigView->setBackgroundPic("sig/sig_wifi_3.png");
					break;
				case 4:
					sigView->setBackgroundPic("sig/sig_wifi_4.png");
					break;
				default:
					break;
			}
		break;
		default:
			sigView->setBackgroundPic("sig/no_sig.png");
		break;
	}
	free(s_netSig);
}

bool DeviceSigManager::wifiIsNormal(){
	return cur_dBm != 0;
}
