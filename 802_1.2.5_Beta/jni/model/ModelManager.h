/*
 * ModelManager.h
 *
 *  Created on: 2020年12月23日
 *      Author: Administrator
 */

#ifndef JNI_MODEL_MODELMANAGER_H_
#define JNI_MODEL_MODELMANAGER_H_

#include "DeviceParamsControl.h"
#include "DownloadManager.h"
#include "control/ZKQRCode.h"

#define SPKEY_SN 					"SPKEY_SN"
#define SPKEY_CHANNEL 				"SPKEY_CHANNEL"
#define SPKEY_PLAYLIST 				"SPKEY_PLAYLIST"
#define SPKEY_PLAYLIST_AND_PARAM 	"SPKEY_PLAYLIST_AND_PARAM"
#define SPKEY_PARAMCHANGE 			"SPKEY_PARAMCHANGE"

#define SPKEY_DEF_VOL 				"SPKEY_DEF_VOL"
#define SPKEY_DEF_QRSIZE 			"SPKEY_DEF_QRSIZE"
#define SPKEY_DEF_QR_DISPLAY 		"SPKEY_DEF_QR_DISPLAY"
#define SPKEY_DEF_CUSTOM_QR 		"SPKEY_DEF_CUSTOM_QR"

using namespace std;

class ModelManager{
public:
	static ModelManager* getInstance();
	void getPlayListAndParams();
	DeviceParamsControl getControl();
	void sendMsgToMqttService(std::string msg);
	typedef struct{
		string type;
		string version;
		DownloadManager::DownloadCallbackFun callback;
		string path;
	} s_upgrade_info;

	void startUpgrade(s_upgrade_info *sUpgradeInfo);
	void startUpgradeApp();
	void parseParamsTask(ZKQRCode* zkQr);

private:
	ModelManager();
};


#define MODELMANAGER ModelManager::getInstance()
#endif /* JNI_MODEL_MODELMANAGER_H_ */
