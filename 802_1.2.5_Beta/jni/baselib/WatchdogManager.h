/*
 * WatchdogManager.h
 *
 *  Created on: 2021年4月10日
 *      Author: Administrator
 */

#ifndef JNI_BASELIB_WATCHDOGMANAGER_H_
#define JNI_BASELIB_WATCHDOGMANAGER_H_


class WatchdogManager{
private:
	int wdt_fd;
	WatchdogManager();
public:
	static WatchdogManager* getInstance();
	bool openWatchdog(int timeout_ms);
	void closeWatchdog();
	void keep_alive();
};
//#define WDG_TIMEOUT_S 300//ethan del 20211208
#define WDG_TIMEOUT_S 120
#define WATCHDOGMANAGER 	WatchdogManager::getInstance()
#endif /* JNI_BASELIB_WATCHDOGMANAGER_H_ */
