/*
 * UrlManager.h
 *
 *  Created on: 2020年12月23日
 *      Author: Administrator
 */

#ifndef JNI_BASELIB_URLMANAGER_H_
#define JNI_BASELIB_URLMANAGER_H_

#include <string>

class UrlManager{
public:

	static UrlManager* getInstance();
	std::string getMqttServerUrl();
	std::string getRequestPlayListAndParamUrl();
	std::string getRequestSnUrl();
	std::string getQrCodeUrl();
	std::string getRequestUpgradeUrl();
	std::string getReportVideolistUrl();
	std::string getConfigSuccessUrl();

private:
	UrlManager();
	bool isTest();
	std::string getBaseUrl();
};

#define URLMANAGER 		UrlManager::getInstance()
#endif /* JNI_BASELIB_URLMANAGER_H_ */
