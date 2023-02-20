/*
 * Module4GManager.h
 *
 *  Created on: 2021年3月6日
 *      Author: Administrator
 */

#ifndef JNI_NET_MODULE4GMANAGER_H_
#define JNI_NET_MODULE4GMANAGER_H_

#include <string>

class Module4GManager{
private:
	Module4GManager();

public:
	static Module4GManager* getInstance();
	void init();
	bool openUart(const char *pFileName, unsigned int baudRate);
	bool sendUartData(const unsigned char* mData, int len);
	std::string getIMEI();
	std::string getICCID();
	std::string getBaseLoc();
	int get4GSig();
	bool isConnectedNet();
};

#define MODULE4GMANAGER 	Module4GManager::getInstance()
#endif /* JNI_NET_MODULE4GMANAGER_H_ */
