/*
 * DeviceManager.cpp
 *
 *  Created on: 2020年12月31日
 *      Author: Administrator
 */

#include "DeviceManager.h"

#include<unistd.h>
#include<sys/reboot.h>

DeviceManager::DeviceManager(){

}

DeviceManager* DeviceManager::getInstance(){
	static DeviceManager mInstance;
	return &mInstance;
}

void DeviceManager::rebootDevice(){
	//同步数据，将缓存数据保存，以防数据丢失
	sync();
	reboot(RB_AUTOBOOT);
}

