/*
 * SNManager.cpp
 *
 *  Created on: 2021年2月1日
 *      Author: Administrator
 */
#include "SNManager.h"
#include "utils/log.h"
#include "../curl/curl.h"
#include "../net/Module4GManager.h"
#include "restclient-cpp/restclient.h"
#include "storage/StoragePreferences.h"
#include "baselib/UrlManager.h"
#include "baselib/utils.h"
#include "baselib/DeviceManager.h"
#include "model/ModelManager.h"
#include "mosquitto-mqtt/mosquitto_sub.h"

ZKQRCode* curQRPtr;

SNManager::SNManager(){
}

SNManager* SNManager::getInstance(){
	static SNManager mInstance;
	return &mInstance;
}

/**
* 线程创建成功后会调用该函数，可以在该函数中做一些初始化操作
* return true   继续线程
*        false  退出线程
*/
bool SNManager::readyToRun() {
 LOGD("getSNThread 已经创建完成\n");
 return true;
}

/**
* 线程循环函数
*
* return true  继续线程循环
*        false 推出线程
*/
bool SNManager::threadLoop() {
	 LOGD("getSNThread - 线程循环函数\n");

	 //为了方便观察，这里添加休眠10s
	 usleep(1000 * 1000 * 10);

	 //检查是否有退出线程的请求，如果有，则返回false，立即退出线程
	 if (exitPending()) {
		 LOGD("getSNThread - 退出线程\n");
	   return false;
	 }
	std::string imei = MODULE4GMANAGER->getIMEI();
	LOGD(" --imei-- %s\n", imei.c_str());
	std::string md5Imei = UTILS->MD5(imei);
	std::string url = URLMANAGER->getRequestSnUrl() + md5Imei;
	LOGD("---------------response.url=%s \n", url.c_str());
	RestClient::Response response = RestClient::get(url);
	LOGD("---------------response.code=%s - %d\n", response.body.c_str(), response.code);
	if(imei.length() > 0 && response.code == 200){
		bool result = JsonParse::parseSN(response.body);
		if(result){
			JsonParse::initThreadDoMqttMsg();
			static string subTopic = "/ibeelink/device/emit/" + md5Imei;
			static string pubTopic = "/ibeelink/device/report/" + md5Imei;
			startMosquittoSub(imei.c_str(), subTopic.c_str(), pubTopic.c_str());
			SNMANAGER->updateQR();
			MODELMANAGER->getPlayListAndParams();
			return false;
		}
	}

	 //返回真，继续下次线程循环
	 return true;
}

void SNManager::startGetSnThread(){
	if(!isRunning()){
		run("getSNThread");
	}
}

void SNManager::initUpdateParam(ZKQRCode* mQRPtr){
	curQRPtr = mQRPtr;
}

void SNManager::updateQR(){
	std::string sn = StoragePreferences::getString(SPKEY_SN, "未找到SN");
	if(sn.size() > 0 && curQRPtr != NULL){
		curQRPtr->loadQRCode((URLMANAGER->getQrCodeUrl() + sn).c_str());
	}
}

