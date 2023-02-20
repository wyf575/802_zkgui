/*
 * DeviceManager.h
 *
 *  Created on: 2020年12月31日
 *      Author: Administrator
 */

#ifndef JNI_BASELIB_DEVICEMANAGER_H_
#define JNI_BASELIB_DEVICEMANAGER_H_

class DeviceManager{
public:
	static DeviceManager* getInstance();
	void rebootDevice();

private:
	DeviceManager();
};


#define DEVICEMANAGER 	DeviceManager::getInstance()
#endif /* JNI_BASELIB_DEVICEMANAGER_H_ */
