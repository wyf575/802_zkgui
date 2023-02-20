/*
 * ModelManager.cpp
 *
 *  Created on: 2020年12月23日
 *      Author: Administrator
 */


#include "ModelManager.h"

#include "restclient-cpp/restclient.h"
#include "baselib/UrlManager.h"
#include "baselib/DeviceManager.h"
#include "JsonParse.h"
#include "storage/StoragePreferences.h"
#include <string>
#include "DeviceParamsControl.h"
#include "utils/Log.h"
#include "baselib/Utils.h"
#include "DownloadManager.h"
#include "include/os/UpgradeMonitor.h"
#include "baselib/FileUtils.h"
#include "../curl/curl.h"
#include "../net/Module4GManager.h"
#include "baselib/TimeUtils.h"
//#include "model/MediaPlayer.h"
#include "appconfig.h"
#include "NetUtils.h"

extern "C"
{
#include "mosquitto-mqtt/mosquitto_sub.h"
}

#include "baselib/Base64Utils.h";


ModelManager::ModelManager(){
}

ModelManager* ModelManager::getInstance(){
	static ModelManager mInstance;
	return &mInstance;
}

void ModelManager::getPlayListAndParams(){

	std::string sn = StoragePreferences::getString(SPKEY_SN, "");
	LOGD(" ----- getPlayListAndParams --- sn:  %s \n", sn.c_str());
	if(sn.length() <= 0){
		return;
	}
	std::string url = URLMANAGER->getRequestPlayListAndParamUrl()+ "?sn="+sn+"&paramType=1";
	RestClient::Response response = RestClient::get(url);
	LOGD(" ----- PlayListAndParams: %s  :  %s \n", url.c_str(), response.body.c_str());
	if(response.code == 200){
		//解析
		JsonParse::parsePlayList(response.body);
		JsonParse::Params params;
		JsonParse::parseParams(response.body, params);
		if(params.paramId > 0 && params.paramList.size() > 0){
			LOGD("参数解析完成\n");
			DEVICEPARAMSCONTROL->initDeviceParams(params);
		}else{
			LOGD("参数解析失败\n");
		}
	}else{
		LOGD("播放列表及参数请求失败！\n");
	}
}

void ModelManager::sendMsgToMqttService(std::string msg){
	std::string strMsg = BASE64UTILS->base64_encode(msg);
	const char* value = strMsg.c_str();
	LOGD("[MQTT]  sendMsgToMqttService - %s\n", value);
	publishMsg(value);
}

void* upgrade(void* sUpgradeInfo){
	ModelManager::s_upgrade_info* sInfo = (ModelManager::s_upgrade_info*)sUpgradeInfo;
	std::string version = sInfo->version;
	LOGD("------upgrade-----%s\n", version.c_str());
	int count = 0;
	while(count < 5){
		string imei = MODULE4GMANAGER->getIMEI();
		std::string url = URLMANAGER->getRequestUpgradeUrl() + "?deviceId=" + UTILS->MD5(imei) + "&version=" + version + "&type=" + sInfo->type;
		LOGD("upgrade: %s\n", url.c_str());
		RestClient::Response response = RestClient::get(url);
		LOGD("------upgrade-----%d --- %s\n", response.code, response.body.c_str());
		if(response.code == 200){
			bool result = JsonParse::parseUpgradeData(response.body, version, sInfo->path, sInfo->callback);
			LOGD("parseUpgradeData.result=%d\n", result);
			if(result){
				break;
			}
		}
		LOGD("升级包请求失败！\n");
		count++;
		sleep(5);
	}
	return NULL;
}

void* upgradeFileDownloadResult(bool result, string path, string version){
	if(result){
		LOGD("升级文件下载成功！\n");
		system("touch ../data/isUpgrade");
		system("sync");
	}else{
		LOGD("升级文件下载失败！\n");
	}
	return NULL;
}

void ModelManager::startUpgrade(ModelManager::s_upgrade_info *sUpgradeInfo){

	pthread_t thread;
	upgrade(sUpgradeInfo);
	// int ret = pthread_create(&thread, NULL, (void*)upgrade, sUpgradeInfo);
	// if(ret){
	// 	printf("[upgradeThread] create error\n");
	// }else{
	// 	printf("[upgradeThread] create success\n");
	// }
}

void ModelManager::startUpgradeApp(){
	LOGD("_____startUpgradeApp______\n");
	static ModelManager::s_upgrade_info sUpgradeInfo;
	sUpgradeInfo.callback = upgradeFileDownloadResult;
	sUpgradeInfo.type = "app";
	sUpgradeInfo.version = APP_VERSION;
	sUpgradeInfo.path = FILEUTILS->getAppUpgardePath();
	startUpgrade(&sUpgradeInfo);
}

void ModelManager::parseParamsTask(ZKQRCode* zkQr){
	
	LOGD("---------parseParamsTask----------------\n");
	if(DEVICEPARAMSCONTROL->getParamId() > 0){

		bool paramChange = StoragePreferences::getBool(SPKEY_PARAMCHANGE, false);
		if(paramChange){
			NetUtils::sendConfigSuccessMsg(DEVICEPARAMSCONTROL->getParamId());
			StoragePreferences::putBool(SPKEY_PARAMCHANGE, false);
		}
		LOGD("设置音量\n");
		int pos = DEVICEPARAMSCONTROL->getVolPercent();
		int defVol = StoragePreferences::getInt(SPKEY_DEF_VOL, -1);
		LOGD("pos=%d   defVol=%d\n", pos, defVol);
		if(pos >= 0 && defVol != pos){
			StoragePreferences::putInt(SPKEY_DEF_VOL, pos);
			//MEDIAPLAYER->setPlayerVol(pos);
		}else if(defVol >= 0){
			//MEDIAPLAYER->setPlayerVol(defVol);
		}

		LOGD("设置二维码大小\n");
		string qrSize = DEVICEPARAMSCONTROL->getQrSize();
		int qrSizeInt = atoi(qrSize.c_str());
		int defQrSize = StoragePreferences::getInt(SPKEY_DEF_QRSIZE, -1);
		LOGD("qrSizeInt=%d   defQrSize=%d\n", qrSizeInt, defQrSize);
		if(qrSize.length() > 0 && defQrSize != qrSizeInt){
			StoragePreferences::putInt(SPKEY_DEF_QRSIZE, qrSizeInt);
			LayoutPosition position = zkQr->getPosition();
			position.mLeft = position.mLeft - (qrSizeInt - position.mWidth);
			position.mHeight = qrSizeInt;
			position.mWidth = qrSizeInt;
			zkQr->setPosition(position);
		}else if(defQrSize > 0){
			LayoutPosition position = zkQr->getPosition();
			position.mLeft = position.mLeft - (defQrSize - position.mWidth);
			position.mHeight = defQrSize;
			position.mWidth = defQrSize;
			zkQr->setPosition(position);
		}

		LOGD("设置二维码显示\n");
		bool isDisplayQr = DEVICEPARAMSCONTROL->isDisplayQRCode();
		bool defQrDisplay = StoragePreferences::getBool(SPKEY_DEF_QR_DISPLAY, true);
		LOGD("isDisplayQr=%d   defQrDisplay=%d\n", isDisplayQr, defQrDisplay);
		if(isDisplayQr && defQrDisplay){
			zkQr->setVisible(isDisplayQr);
		}else{
			StoragePreferences::putBool(SPKEY_DEF_QR_DISPLAY, isDisplayQr);
			zkQr->setVisible(isDisplayQr);
		}

		LOGD("设置自定义二维码\n");
		string qrLink = DEVICEPARAMSCONTROL->displayCustomQRCode();
		string defCustomQr = StoragePreferences::getString(SPKEY_DEF_CUSTOM_QR, "");
		LOGD("qrLink=%s   defCustomQr=%s\n", qrLink.c_str(), defCustomQr.c_str());
		if(qrLink.length() > 0 && qrLink != defCustomQr){
			StoragePreferences::putString(SPKEY_DEF_CUSTOM_QR, qrLink);
			zkQr->loadQRCode(qrLink.c_str());
		}else if(defCustomQr.length() > 0){
			zkQr->loadQRCode(defCustomQr.c_str());
		}
	}
}

