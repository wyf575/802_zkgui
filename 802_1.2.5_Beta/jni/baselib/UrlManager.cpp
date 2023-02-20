/*
 * UrlManager.cpp
 *
 *  Created on: 2020年12月23日
 *      Author: Administrator
 */

#include "UrlManager.h"
#include <String>
#include "appconfig.h"

UrlManager::UrlManager(){
}

std::string UrlManager::getBaseUrl(){
	if(IS_TEST_MODE){
		return "http://tmp.ibeelink.com/";
	}else{
		return "http://mqtt.ibeelink.com/";
	}
}

UrlManager* UrlManager::getInstance(){
	static UrlManager mInstance;
	return &mInstance;
}

std::string UrlManager::getRequestPlayListAndParamUrl(){
	return getBaseUrl() + "api/ad/play/list-param";
}

std::string UrlManager::getQrCodeUrl() {
	return getBaseUrl() + "popularize?sn=";
}

std::string UrlManager::getRequestSnUrl(){
	return getBaseUrl() + "api/device/exchange/sn?deviceId=";
}

std::string UrlManager::getRequestUpgradeUrl(){
	return getBaseUrl() + "api/version/upgrade/did";
}

std::string UrlManager::getReportVideolistUrl(){
	return getBaseUrl() + "api/ad/report";
}

std::string UrlManager::getConfigSuccessUrl(){
	return getBaseUrl() + "api/config/success";
}
