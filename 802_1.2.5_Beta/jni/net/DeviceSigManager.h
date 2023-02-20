/*
 * DeviceSigManager.h
 *
 *  Created on: 2021年2月19日
 *      Author: Administrator
 */

#ifndef JNI_MODEL_DEVICESIGMANAGER_H_
#define JNI_MODEL_DEVICESIGMANAGER_H_

#include "hotplugdetect.h"
#include <string>
#include "control/ZKTextView.h"

class DeviceSigManager{
private:
	DeviceSigManager();
public:
	typedef struct{
		int sig;
		int sigType;
	}S_NetSig;
	static DeviceSigManager* getInstance();
	static void WifiConnStatusCallback(char *pSsid, int status, int quality);
	static void WifiSignalStatusCallback(ScanResult_t *pstScanResult, int resCnt);
	int getNetType();
	bool wifiIsNormal();

	S_NetSig* getNetSignal();
	void updateSigUI(ZKTextView* sigView);

};

#define DEVICESIGMANAGER 	DeviceSigManager::getInstance()
#endif /* JNI_MODEL_DEVICESIGMANAGER_H_ */
